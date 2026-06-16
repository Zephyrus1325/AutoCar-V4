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
#include "navigation/motors/motor_task.h"   // To get motor data
#include "sensors/lidar/lidar_task.h"       // To get lidar readings


AsyncWebServer server(NETWORK_SERVER_PORT);
AsyncWebSocket ws("/ws");

TaskHandle_t telemetry_handler;

void sendMotorData(){
    JsonDocument doc;
    doc["type"] = "telemetry";
    doc["motor"]["left"]["speed"] = left_motor.getRPM();
    doc["motor"]["left"]["setpoint"] = left_motor.getSetpoint();
    doc["motor"]["left"]["throttle"] = left_motor.getThrottle() * 100;
    doc["motor"]["right"]["speed"] = right_motor.getRPM();
    doc["motor"]["right"]["setpoint"] = right_motor.getSetpoint();
    doc["motor"]["right"]["throttle"] = right_motor.getThrottle() * 100;
    String out;
    serializeJson(doc, out);
    
    ws.textAll(out);
}

void sendLidarData(){
    JsonDocument doc;
    LidarData data = lidar.read();
    doc["type"] = "telemetry";
    doc["lidar"]["rpm"] = data.rpm();
    doc["lidar"]["setpoint"] = lidar.getSetPoint();
    doc["lidar"]["throttle"] = (float)((float)data.throttle())/10.23f;

    String out;
    serializeJson(doc, out);
    
    ws.textAll(out);
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
    
    uint64_t motor_wait = 100;
    
    while(true){
        if(millis() - last_motor > motor_wait ){
            sendMotorData();
            sendLidarData();
            last_motor = millis();
        }
        delay(1);
    }
}
