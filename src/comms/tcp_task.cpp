#include "comms/udp_task.h"
#include "common/configs.h"


#include "comms/network_common.h"
#include "comms/network_config.h"

#include "comms/network_task.h"

TaskHandle_t tcp_task_handler;

typedef struct {
    uint16_t length;
    uint16_t type;
    uint8_t data[];
} __attribute__((packed)) tcp_header_t;

typedef struct {
    int16_t accel_offset_x;
    int16_t accel_offset_y;
    int16_t accel_offset_z;
    int16_t gyro_offset_x;
    int16_t gyro_offset_y;
    int16_t gyro_offset_z;

    float accel_gain_x;
    float accel_gain_y;
    float accel_gain_z;
    float gyro_gain_x;
    float gyro_gain_y;
    float gyro_gain_z;

    uint8_t low_pass_filter;
    uint8_t accel_range;
    uint8_t gyro_range;
} __attribute__((packed)) imu_param_t;

typedef struct {
    float leftP;
    float leftI;
    float leftD;
    float rightP;
    float rightI;
    float rightD;
} __attribute__((packed)) motor_param_t;

#include "sensors/lidar/lidar_task.h"
#include "navigation/motors/motor_task.h"

void parse_packet(void* packet, size_t len, AsyncClient* client){
    uint8_t* ptr = (uint8_t*) packet;
    size_t offset = 0;

    //Serial.println("Address: " + client->getRemoteAddress());
    
    while(offset < len){
        tcp_header_t* header = (tcp_header_t*)(ptr + offset);
        
        //Serial.println("Header: " + header->type);

        if(header->type == UDP_PACKET_COMMAND){
            switch(header->data[0]){
                case 'q':
                    motor.setRPM(20);
                    break;
                case 'w':
                    motor.setRPM(40);
                    break;
                case 'e':
                    motor.setRPM(60);
                    break;
                case 'r':
                    motor.setRPM(80);
                    break;
                case 't':
                    motor.setRPM(100);
                    break;
                case 'y':
                    motor.setRPM(110);
                    break;
                case 'u':
                    motor.setRPM(120);
                    break;
                case 'i':
                    motor.setRPM(130);
                    break;
                case 'z':
                    motor.setRPM(0);
                    break;
                case honk:
                    //Serial.println("Honk");
                    break;
                case setIMULowPassFilter:        // uint8_t (0-6)
                    break;
                case setAccelSampleRate:         // uint8_t (0-4)
                    break;
                case setGyroSampleRate:          // uint8_t (0-4)
                    break;
                case setLidarRpm:                // uint16_t (0-450)
                    break;
                case setLidarSampleRate:         // uint8_t (16-32)
                    break;
                case requestIMUConfig:           // None
                    break;
                case requestNetworkConfig:       // None
                    break;
                case requestNavigationConfig:    // None
                    break;
                default:
                    break;
            }
        }

        offset += header->length;
    }
}


void tcp_task(void* param){

    tcp = new AsyncServer(TCP_COMMAND_PORT);

    tcp->onClient([](void *arg, AsyncClient* c){
        if(c == NULL){
            return;
        }

        // If someone else tries to connect, reject it
        if(remote != NULL && remote->connected()){
            c->close();
            return;
        }

        remote = c;
        xTaskNotifyGive(network_handler);   // Notify connection success so Networker starts looking its health

        Serial.println("Client from " + c->remoteIP().toString());

        // Callback para quando chegarem dados do Processing
        remote->onData([](void *arg, AsyncClient *c, void *data, size_t len) {
            // Notifique sua task de controle aqui se necessário
            parse_packet(data, len, c);
        }, NULL);

        // Callback para desconexão
        remote->onDisconnect([](void *arg, AsyncClient *c) {
            Serial.println("Client Disconnected");
            remote = NULL;
        }, NULL);

    }, NULL);

    tcp->begin();

    while(true){
        
        // Waits for main networking to give a ip


        yield();
    }
}