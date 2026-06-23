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

void sendChunkData(uint8_t id){
    
    if (active_chunks == nullptr) return;

    if (active_chunks[id] == nullptr || active_chunks[id]->data == nullptr) {
        return; 
    }   

    uint32_t data_size = sizeof(chunk_t) + active_chunks[id]->size * active_chunks[id]->size;
    ws.binaryAll((uint8_t*) active_chunks[id], data_size);
    yield();    // give some time so other tasks can breathe
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
    doc["motor"]["left"]["setpoint"] = motor.left.setpoint;
    doc["motor"]["left"]["throttle"] = motor.left.throttle;
    doc["motor"]["right"]["speed"] = motor.right.linear_velocity;
    doc["motor"]["right"]["setpoint"] = motor.right.setpoint;
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

    lidar_packet packet = getLidarData();

    // Send packet data
    int16_t len = packet.num_samples;

    float binary_out[64];

    uint16_t* data_id = (uint16_t*) binary_out;
    *data_id = 1;                           // Sets id to 1 (Lidar data)g

    binary_out[1] = len;
    binary_out[2] = packet.angle[0] + 90.f; // TODO: send the correct angle/distance relation

    for(int i = 0; i < len; i++){
       binary_out[3+i] = packet.distances[i];
    }

    ws.binaryAll((const char*) binary_out, sizeof(float) * (len + 3));

    free(packet.angle);
    free(packet.distances);
}

void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        JsonDocument json;

        deserializeJson(json, data);
        
        if(json["type"] == "ping"){
            JsonDocument msg;
            msg["type"] = "pong";
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
            //left_motor.setRPM(velocidadeEsquerda);
            //right_motor.setRPM(velocidadeDireita);
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
        //left_motor.setRPM(0);       // TEMPORARY STUFF, REMOVE LATER -----------------------------------
        //right_motor.setRPM(0);      // TEMPORARY STUFF, REMOVE LATER -----------------------------------
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
    uint64_t motor_wait = 50;

    uint64_t last_chunk = 0;
    uint64_t chunk_wait = 2000;

    uint64_t last_lidar = 0;
    uint64_t lidar_wait = 100;
    uint8_t lidar_id = 0;

    bool lidar_available = false;
    subscribe_lidar(&lidar_available);

    uint8_t chunk_id = 0;

    while(true){
        if(ws.count() > 0){
            if(millis() - last_motor > motor_wait){
                sendMotorData();
                sendPositionData();
                last_motor = millis();
            }

            if(millis() - last_chunk > chunk_wait){
                //sendChunkData(chunk_id++);
                chunk_id = chunk_id > 4 ? 0 : chunk_id;
                last_chunk = millis();
            }
            
            if(millis() - last_lidar > lidar_wait){
                if(lidar_available){
                    sendLidarData(); 
                    lidar_available = 0;
                    lidar_id++;
                }
                if(lidar_id > 16){last_lidar = millis(); lidar_id = 0;}
            }   
            
            delay(NETWORK_DELAY);
            //yield();
        } else {
            delay(1000);
        }
    }
}
