#include "comms/udp_task.h"
#include "common/configs.h"
#include "common/queues.h"

#include "comms/network_common.h"
#include "comms/network_config.h"

#include "sensors/lidar/lidar_task.h"

TaskHandle_t udp_task_handler;

// UDP Speficic packets

typedef struct {
    uint8_t msgType;
    uint16_t rpm;
    uint16_t target_rpm;
    uint16_t throttle;
    float initialAngle;
    uint16_t num_readings;
    float readings[35];
} __attribute__((packed)) lidar_data_t;

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} __attribute__((packed)) imu_reading_t;



typedef struct {
    int16_t leftSpeed;      // Times 10
    int16_t leftSetpoint;   // Times 10
    int16_t leftThrottle;   // -255 to 255

    int16_t rightSpeed;      // Times 10
    int16_t rightSetpoint;   // Times 10
    int16_t rightThrottle;   // -255 to 255
} __attribute__((packed)) motor_data_t;



void sendLidarData(){
    static uint32_t last_id = 0;
    
    LidarData l;
    l = lidar.read();

    if(last_id != l.id()){
        lidar_data_t data;
        data.rpm = l.rpm();
        data.target_rpm = l.setPoint();
        data.throttle = l.throttle();
        data.msgType = l.msgType();

        uint16_t msgLen = 7;
        if(l.msgType() == 0xAD){
            data.initialAngle = l.initialAngle();
            data.num_readings = l.length();

            for(int i = 0; i < l.length(); i++){
                data.readings[i] = l.distance(i);
            }
                
            msgLen = sizeof(float) * l.length() + 13;
        }

        udp_lidar.writeTo((uint8_t*)&data, msgLen, remote_pc_ip, UDP_LIDAR_PORT);
        last_id = l.id();
    }
}

void sendChunkData(){
    // Does nothing, for now :D
}

#include "Lidar.h"

extern Lidar lidar;

void udp_task(void* param){
    if(udp_chunk.listen(UDP_CHUNK_PORT)) {
        udp_chunk.onPacket([](AsyncUDPPacket packet) {});
    }

    if(udp_lidar.listen(UDP_LIDAR_PORT)) {
        udp_lidar.onPacket([](AsyncUDPPacket packet) {});
    }

    if(udp_telemetry.listen(UDP_TELEMETRY_PORT)) {
        udp_telemetry.onPacket([](AsyncUDPPacket packet) {
            // This runs whenever a message arrives
            if(packet.data()[0] == UDP_STREAM_HANDSHAKE){
                String handshake_msg = DISCOVERY_MESSAGE;

                if(handshake_msg.equals((const char*) &packet.data()[1])){
                     
                    // Reply to sender
                    uint8_t msg[17 + 1];
                    msg[0] =  UDP_STREAM_HANDSHAKE;
                    strcpy((char*) &msg[1], CALLBACK_MESSAGE);

                    AsyncUDPMessage reply;
                    reply.write(msg, 17 + 1);
                    remote_pc_ip = packet.remoteIP();
                    udp_telemetry.sendTo(reply, remote_pc_ip, UDP_TELEMETRY_PORT);                    
                } 
            }   
        });
    }


    while(true){
        if(remote != NULL){
            sendLidarData();        // Check if theres new lidar data to send
            sendChunkData();        // Does nothing right now
            vTaskDelay(1);          // Check data to send every 1 ms
        } else {
            vTaskDelay(1000);
        }
    }
}

