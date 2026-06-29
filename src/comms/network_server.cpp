#include "network_server.h"

#include "common/configs.h"
#include "util/logger.h"

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ESPmDNS.h>

// Entities to get
#include "common/motor_data.h"      // To get motor data
#include "common/lidar_data.h"      // To get lidar readings
#include "common/imu_data.h"        // Imu Readings
#include "common/robot_kinematics.h" // To get robot position and speed data

#include "navigation/chunks/chunk.h"


AsyncWebServer server(NETWORK_SERVER_PORT);
AsyncWebSocket ws("/ws");

TaskHandle_t telemetry_handler;
lidar_packet* lidar_data;   // Packet used to send lidar data

// TODO: Add more attributes to the header (too lazy to add more data rn)
struct __attribute__((packed)) ChunkHeader {
    uint16_t dataType;  // 2 bytes (ex: valor 2 para Chunk)
    int16_t chunkX;     // 2 bytes
    int16_t chunkY;     // 2 bytes
    uint16_t part;  // 2 bytes (tamanho dos dados que vêm atrás)
};

void sendChunkData(uint8_t id, uint8_t part, uint8_t* buffer){
    if (buffer == nullptr) return;
    if (id >= 4) return;
    if (part >= 16) return;

    // 1. Tenta pegar o mutex de forma única. Aguarda até 5ms se estiver ocupado.
    if (xSemaphoreTake(chunk_mutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        
        // --- SEÇÃO CRÍTICA PROTEGIDA ---
        // Se qualquer validação falhar, precisamos garantir que o mutex seja liberado!
        if (active_chunks == nullptr || 
            active_chunks[id] == nullptr || 
            active_chunks[id]->data == nullptr) {
            
            xSemaphoreGive(chunk_mutex); // Libera o Mutex antes de sair!
            return;
        }

        const uint32_t total_map_size = CHUNK_SIZE * CHUNK_SIZE;
        uint32_t slice_size = total_map_size / 16;
        uint32_t part_index = slice_size * part; 

        if (part == 15) {
            slice_size = total_map_size - part_index;
        }

        // Preenche o cabeçalho no buffer local
        ChunkHeader* header = (ChunkHeader*) buffer;
        header->dataType = 2; 
        header->chunkX = active_chunks[id]->x;
        header->chunkY = active_chunks[id]->y;
        header->part = part;

        // Executa a cópia da RAM do chunk para o buffer local de transmissão
        if (part_index + slice_size <= total_map_size) {
            memcpy(buffer + sizeof(ChunkHeader), &active_chunks[id]->data[part_index], slice_size);
        }

        // 计算 Pacote Completo
        const uint32_t data_size = sizeof(ChunkHeader) + slice_size;

        // --- FIM DA SEÇÃO CRÍTICA ---
        xSemaphoreGive(chunk_mutex); // Solta o mutex imediatamente!

        // 2. Agora que os dados já estão salvos e isolados no seu buffer local,
        // você pode transmitir via rede sem prender o resto do robô.
        ws.binaryAll(buffer, data_size);
        yield();
        
    } else {
        // Se o mutex estava ocupado com a Task de geração do mapa,
        // apenas pula esse envio para não travar o FreeRTOS.
        return; 
    }
}

void sendPositionData(){
    JsonDocument doc;

    robot_kinematic kinematics = getKinematics();

    doc["type"] = "telemetry";
    doc["navigation"]["position"]["x"] = kinematics.position.x;
    doc["navigation"]["position"]["y"] = kinematics.position.y;
    doc["navigation"]["position"]["heading"] = kinematics.position.heading;

    String out;
    serializeJson(doc, out);
    
    ws.textAll(out);
}

void sendMotorData(){
    JsonDocument doc;

    motor_data motor = getMotorData();

    doc["type"] = "telemetry";
    doc["motor"]["left"]["speed"] = motor.left.linear_velocity;
    doc["motor"]["left"]["setpoint"] = motor.left.setpoint * PI * WHEEL_DIAMETER / 60.f;
    doc["motor"]["left"]["throttle"] = motor.left.throttle;
    doc["motor"]["right"]["speed"] = motor.right.linear_velocity;
    doc["motor"]["right"]["setpoint"] = motor.right.setpoint * PI * WHEEL_DIAMETER / 60.f;
    doc["motor"]["right"]["throttle"] = motor.right.throttle;
    String out;
    serializeJson(doc, out);
    
    ws.textAll(out);
}

void sendLidarData(){
    // Send basic telemetry info
    lidar_info info = getLidarInfo();
    
    JsonDocument doc;
    
    doc["type"] = "telemetry";
    doc["lidar"]["rpm"] = info.rpm;
    doc["lidar"]["setpoint"] = info.setpoint;
    doc["lidar"]["throttle"] = info.throttle;
    //print_log(String(info.rpm) + " | " + String(info.setpoint) + " | " + String(info.throttle));
    String out;
    
    serializeJson(doc, out);
    ws.textAll(out);

    // Send actual telemetry, if theres any
    if(info.type != LIDAR_NORMAL){return;}

    getLidarData(lidar_data);

    // Send packet data
    int16_t len = lidar_data->num_samples;

    ws.binaryAll((const char*) lidar_data, sizeof(lidar_packet) + sizeof(float) * len * 2);
}

void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        JsonDocument json;

        deserializeJson(json, data);
        
        if(json["type"] == "ping"){
            JsonDocument msg;
            msg["type"] = "pong";
            msg["battery"] = BATTERY_CALIB_Y0 + (analogRead(BATTERY_PIN) - BATTERY_CALIB_X0) * (BATTERY_CALIB_Y1 - BATTERY_CALIB_Y0)/(BATTERY_CALIB_X1 - BATTERY_CALIB_X0);
            if(WiFi.getMode() == WIFI_STA){
                msg["rssi"] = WiFi.RSSI();
            } else {
                msg["rssi"] = "--";
            }
            
            String output;
            serializeJson(msg, output);

            ws.text(client->id(), output);
        } else if (json["type"] == "cmd_vel") {
            float velocidadeEsquerda = json["left"];
            float velocidadeDireita = json["right"];

            // Aplica diretamente o Setpoint que o seu loop de PID vai ler no próximo update()
            setMotorSpeed(velocidadeEsquerda, velocidadeDireita);
        }
    }

    
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
        print_log("Client connected from " + client->remoteIP().toString());
        break;
    case WS_EVT_DISCONNECT:
        print_alert("Client Disconnected.");
        setMotorSpeed(0, 0);
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(client, arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_PING:
    case WS_EVT_ERROR:
        break;
  }
}


void startServer(){

    // Set mDNS
    if (!MDNS.begin(MDNS_NAME)) {
        print_log("Error setting up MDNS responder");
    } else {
        print_log("MDNS started.");
    }

    // Add service to MDNS-SD
    MDNS.addService("_http", "_tcp", NETWORK_SERVER_PORT);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/index.html", "text/html");
    });
    server.serveStatic("/", LittleFS, "/");

    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.begin();
    print_log("Web server started.");
}

void telemetry_task(void *args){
    uint64_t last_motor = 0; 
    uint64_t motor_wait = 27;

    uint64_t last_chunk = 0;
    uint64_t chunk_wait = 49;

    uint64_t last_lidar = 0;
    uint64_t lidar_wait = 102;
    uint8_t lidar_id = 0;

    bool lidar_available = false;
    subscribe_lidar(&lidar_available);
    lidar_data = (lidar_packet*) malloc(sizeof(lidar_packet) + 40 * sizeof(float) * 2); 

    uint8_t chunk_id = 0;
    uint8_t chunk_part = 0;

    const uint32_t data_size = sizeof(ChunkHeader) + (CHUNK_SIZE * CHUNK_SIZE) / 16;
    uint8_t* chunk_buffer = (uint8_t*) malloc(data_size);

    while(true){
        if(ws.count() > 0){
            if(millis() - last_motor > motor_wait){
                sendMotorData();
                sendPositionData();
                last_motor = millis();
            }
            
            if(millis() - last_lidar > lidar_wait){
                if(lidar_available){
                    sendLidarData(); 
                    lidar_available = 0;
                    lidar_id++;
                }
                if(lidar_id > 16){last_lidar = millis(); lidar_id = 0;}
            }   

            if(millis() - last_chunk > chunk_wait){
                sendChunkData(chunk_id, chunk_part++, chunk_buffer);
                if(chunk_part >= 16){
                    chunk_part = 0;
                    chunk_id++;
                    chunk_id = chunk_id > 4 ? 0 : chunk_id;
                }
                
                last_chunk = millis();
            }
            
            delay(NETWORK_DELAY);
            //yield();
        } else {
            delay(1000);
        }
    }
}
