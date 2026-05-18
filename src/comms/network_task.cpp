#include "comms/network_task.h"
#include "comms/network_config.h"
#include "common/configs.h"

#include "comms/udp_task.h"
#include "comms/tcp_task.h"
#include "comms/network_common.h"

char sta_ssid[32] = "Marco_2G";             // 
char sta_password[32] = "MarcoFilho12";     // TODO: GET THAT INFO FROM THE SD CARD WHEN THE MEMORY TASK GETS DONE
char ap_ssid[32] = "AutoCar_V4";            // 
char ap_password[32] = "VroomVroom";        // 

TaskHandle_t network_handler;

// Tries to connect to a router
void set_sta(){
    WiFi.disconnect();      // Disconnects from last WiFi access
    WiFi.mode(WIFI_STA);
    WiFi.begin(sta_ssid, sta_password);
    uint32_t delay_cycles = 50;        // Variable to measure how much time we waited and timeout otherwise
    while((WiFi.status() != WL_CONNECTED) && delay_cycles > 0){
        delay(50);          // Waits a bit to allow for the connection to be established
        delay_cycles--;
    }
}

// Sets up ourselves as a router
void set_ap(){
    WiFi.disconnect();      // Disconnects from last WiFi access
    WiFi.mode(WIFI_AP);     
    WiFi.softAP(ap_ssid, ap_password);  // Starts the access point
}

// Task responsible for network management
void network_task(void* param){
    // Setup Initial WiFi Status
    // Tries to connect to saved ssid
    // If it fails, enters STA mode

    set_sta();
    if(WiFi.status() != WL_CONNECTED){
        set_ap();
        Serial.println("Couldn't connect to WiFi, backing up with Access Point.");
    }

    WiFi.setSleep(false); // Disables power saving completely

    // Starts the other networking tasks

    xTaskCreatePinnedToCore(tcp_task, "TCP Networking Task", UDP_TRANSMIT_STACK_SIZE, NULL, 1, &tcp_task_handler, NETWORK_CORE);
    xTaskCreatePinnedToCore(udp_task, "UDP Networking Task", TCP_TRANSMIT_STACK_SIZE, NULL, 1, &udp_task_handler, NETWORK_CORE);

    // Keeps checking for a the connection
    while(true){
        vTaskDelay(portMAX_DELAY); // Stops the task, for now
        
        //// If it loses connection with WiFi, stops others tasks, and tries to reconnect
        //uint32_t reconnect_tries = 0;
        //while(WiFi.status() != WL_CONNECTED && reconnect_tries < MAX_RECONNECT_TRIES){
        //    WiFi.disconnect();
        //    WiFi.reconnect();
        //    reconnect_tries++;
        //    vTaskDelay(1000);
        //}   // If it takes too much time, give up and wait for a new connection
        
        
    } // Infinite Loop
}

