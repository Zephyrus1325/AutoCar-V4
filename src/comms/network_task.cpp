#include "comms/network_task.h"
#include "common/configs.h"
#include "network_util.h"
#include "util/logger.h"
#include "util/file_read.h"

#include <WiFi.h>
#include "network_server.h"

TaskHandle_t network_handler;
QueueHandle_t console_queue;

void network_print(String str){
    print_alert("Function \"<network_print()\" not implemented.");
}

void scan_ssid(){
    Serial.println("Scan start");
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  WiFi.scanDelete();
  delay(5000);
}

// Tries to connect to a router
void set_sta(){
    JsonDocument config = getConfigData();

    const char* ssid = config["wifi"]["ssid"];
    const char* password = config["wifi"]["password"];

    WiFi.disconnect();      // Disconnects from last WiFi access
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    uint32_t delay_cycles = 500;        // Variable to measure how much time we waited and timeout otherwise
    while((WiFi.status() != WL_CONNECTED) && delay_cycles > 0){
        delay(50);          // Waits a bit to allow for the connection to be established
        delay_cycles--;
    }
}

// Sets up ourselves as a router
void set_ap(){
    JsonDocument config = getConfigData();

    const char* ssid = config["ap"]["ssid"];
    const char* password = config["ap"]["password"];

    WiFi.disconnect();      // Disconnects from last WiFi access
    WiFi.mode(WIFI_AP);     
    WiFi.softAP(ssid, password);  // Starts the access point
}

// Task responsible for network management
void network_task(void* param){
    // Setup Initial WiFi Status
    // Tries to connect to saved ssid
    // If it fails, enters STA mode
    set_sta();
    if(WiFi.status() != WL_CONNECTED){
        set_ap();
        print_log("Couldn't connect to WiFi, backing up with Access Point - AP IP: " + WiFi.softAPIP().toString());
    } else {
        print_log("Connected to WiFi Router - Local IP: " + WiFi.localIP().toString());
    }

    WiFi.setSleep(false); // Disables power saving completely

    delay(1000);          // Adds a user-unnoticible wait so WiFi is garanteed stable
    startServer();        // Starts server
    xTaskCreatePinnedToCore(telemetry_task, "Telemetry Task", 4096, NULL, 1, &telemetry_handler, NETWORK_CORE);

    // Keeps checking for a the connection
    while(true){
        if(WiFi.getMode() == WIFI_STA && !WiFi.isConnected()){
            print_log("WiFi Connection Lost");
            set_sta();  // Try Connecting again
        }
        vTaskDelay(1000); // Wait a while before checking up again     
    }
}   

