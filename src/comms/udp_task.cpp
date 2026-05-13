#include "comms/udp_task.h"
#include "common/configs.h"
#include "common/queues.h"

#include "comms/network_common.h"
#include "comms/network_config.h"

#include "sensors/lidar/lidar_task.h"

TaskHandle_t udp_task_handler;

// UDP Speficic packets

typedef struct {
    uint8_t packetType = lidarData;
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

        uint16_t msgLen = 8;
        if(l.msgType() == 0xAD){
            data.initialAngle = l.initialAngle();
            data.num_readings = l.length();

            for(int i = 0; i < l.length(); i++){
                data.readings[i] = l.distance(i);
            }
                
            msgLen = sizeof(float) * l.length() + 14;
        }
        udp.writeTo((uint8_t*)&data, msgLen, remote_pc_ip, UDP_CHUNK_PORT);
        last_id = l.id();
    }
}

void sendChunkData(){
    // Does nothing, for now :D
}

#include "Lidar.h"

extern Lidar lidar;
uint16_t rpm = 180;

void udp_task(void* param){
    if(udp.listen(UDP_CHUNK_PORT)){
        udp.onPacket([](AsyncUDPPacket packet) {
            // Code that run for each received UDP packet
            /*  Algorithm:
             *  - PC sends a hello message broadcast with his ip
             *  - ESP receives it, and stores the new ip, if not connected to another on already
             *  (Yeah, for now we allow only one computer to receive the telemetry)
             *  (Maybe allow for more IPs when the project gets bigger?) - Search about Multicast for that
             */


            // if theres no connected pc yet, run this
            // if there is, just ignore the packet :D
            if(remote == NULL){

                // Read Message Type
                if(packet.data()[0] == handshakeMessage){
                    
                    String discovery_msg = (const char*)(packet.data() + 1);
                    // If the message is correct, set the new remote IP
                    if(discovery_msg.equals(UDP_DISCOVERY_MESSAGE)){
                        remote_pc_ip = packet.remoteIP(); 
                        // Send a confirmation message so the remote pc knows we exist in the network
                        // Also this gives our IP to them
                        String msg = String(UDP_CONFIRM_MESSAGE);

                        uint8_t buffer[msg.length() + 1];
                        buffer[0] = handshakeMessage;
                        memcpy(&buffer[1], msg.c_str(), msg.length());
                        udp.writeTo(buffer, msg.length()+1, remote_pc_ip, UDP_CHUNK_PORT);  
                    }
                } 

            } else if (packet.data()[0] == commandStream){
                    uint8_t data = packet.data()[1];
                    switch(data){
                        case 's':
                            digitalWrite(MOTOR_LEFT_A_PIN, HIGH);
                            digitalWrite(MOTOR_LEFT_B_PIN, LOW);
                            digitalWrite(MOTOR_RIGHT_A_PIN, HIGH);
                            digitalWrite(MOTOR_RIGHT_B_PIN, LOW);
                            break;
                        case 'w':
                            digitalWrite(MOTOR_LEFT_A_PIN, LOW);
                            digitalWrite(MOTOR_LEFT_B_PIN, HIGH);
                            digitalWrite(MOTOR_RIGHT_A_PIN, LOW);
                            digitalWrite(MOTOR_RIGHT_B_PIN, HIGH);                            
                            break;
                        case 'a':
                            digitalWrite(MOTOR_LEFT_A_PIN, HIGH);
                            digitalWrite(MOTOR_LEFT_B_PIN, LOW);
                            digitalWrite(MOTOR_RIGHT_A_PIN, LOW);
                            digitalWrite(MOTOR_RIGHT_B_PIN, HIGH);   
                            break;
                        case 'd':
                            digitalWrite(MOTOR_LEFT_A_PIN, LOW);
                            digitalWrite(MOTOR_LEFT_B_PIN, HIGH);
                            digitalWrite(MOTOR_RIGHT_A_PIN, HIGH);
                            digitalWrite(MOTOR_RIGHT_B_PIN, LOW); 
                            break;
                        case 'q':
                            lidar.setRPM(--rpm);
                            break;
                        case 'e':
                            lidar.setRPM(++rpm);
                            break;
                        default:
                            digitalWrite(MOTOR_LEFT_A_PIN, LOW);
                            digitalWrite(MOTOR_LEFT_B_PIN, LOW);
                            digitalWrite(MOTOR_RIGHT_A_PIN, LOW);
                            digitalWrite(MOTOR_RIGHT_B_PIN, LOW);
                            break;

                    }
            }
        });
    }

    while(true){
        //checkReceivedUDP();   // Checks if theres new UDP packets incoming

        sendLidarData();        // Check if theres new lidar data to send
        sendChunkData();        // Does nothing right now
        vTaskDelay(5);          // Check data to send every 5 ms
    }
}

