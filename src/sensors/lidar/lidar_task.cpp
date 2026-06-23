#include "sensors/lidar/lidar_task.h"
#include "common/configs.h"
#include "common/queues.h"
#include "util/file_read.h"
#include "util/logger.h"

#include "Lidar.h"
#include "common/lidar_data.h"

SemaphoreHandle_t lidar_mutex;
SemaphoreHandle_t info_mutex;
TaskHandle_t lidar_task_handler;
Lidar lidar;

static lidar_packet packet;
static lidar_info info;

static bool* subscribers[20];
static uint16_t total_subscribers = 0;

void alert_available(){
    for(int i = 0; i < total_subscribers; i++){
        *subscribers[i] = true;
    }
}

void lidar_task(void* param){
    lidar.begin(&Serial1, LIDAR_MOTOR_PIN);
    
    // Allocate final shared processed data
    packet.angle = (float*) malloc(40 * sizeof(float));     
    packet.distances = (float*) malloc(40 * sizeof(float));

    // Allocate temporaty processing data
    lidar_packet temp;
    temp.angle = (float*) malloc(40 * sizeof(float));     
    temp.distances = (float*) malloc(40 * sizeof(float));

    lidar_info temp_info;

    // Get lidar config from file
    JsonDocument config = getConfigData();
    uint16_t rpm = config["lidar"]["rpm"];

    lidar.setMotor(500);
    delay(100);
    lidar.setRPM(rpm);
    
    uint16_t last_id = 0;

    while(true){
        lidar.update();         // check if there are readings
        LidarData reading = lidar.read();
        if(reading.id() != last_id){
            last_id = reading.id();

            // Check if its a valid usable reading
            if(reading.length() > 0){
                // fill with new data
                temp.num_samples = reading.length();

                float initial_angle = reading.initialAngle();

                // Note: for now just repeating same data
                // TODO: Later on add unskewing for the data
                for(int i = 0; i < temp.num_samples; i++){
                    temp.angle[i] = initial_angle + i * (22.5f / temp.num_samples);
                    temp.distances[i] = reading.distance(i);
                }
        
                // Update with new data
                if(xSemaphoreTake(lidar_mutex, 5) == pdTRUE){
                    packet.num_samples = temp.num_samples;

                    memcpy(packet.angle, temp.angle, temp.num_samples * sizeof(float));
                    memcpy(packet.distances, temp.distances, temp.num_samples * sizeof(float));
                    xSemaphoreGive(lidar_mutex);
                }

                temp_info.type = LIDAR_NORMAL;
                
            } else {
                temp_info.type = LIDAR_WRONG_SPEED;
            }

            // Also send the metadate info
            temp_info.rpm = reading.rpm();
            temp_info.setpoint = reading.setPoint();
            temp_info.throttle = reading.throttle() / 1023.f;

            if(xSemaphoreTake(info_mutex, 5) == pdTRUE){
                info = temp_info;
                xSemaphoreGive(info_mutex);
            }

            alert_available();    // Alert other tasks about the new available data
            continue;   // Try to read another packet so they wont pile up
        }
        
        delay(LIDAR_DELAY);               // Lidar is quite slow, so we can check for new data with slower frequency
    }
}

// Adds a flag to a subscriber list
void subscribe_lidar(bool* flag_ptr){
    subscribers[total_subscribers++] = flag_ptr;
}

lidar_packet getLidarData(){
    lidar_packet out_packet;
    if(xSemaphoreTake(lidar_mutex, 5) == pdTRUE){
        out_packet.num_samples = packet.num_samples;

        out_packet.angle = (float*) malloc(packet.num_samples * sizeof(float));
        out_packet.distances = (float*) malloc(packet.num_samples * sizeof(float));

        memcpy(out_packet.angle, packet.angle, packet.num_samples * sizeof(float));
        memcpy(out_packet.distances, packet.distances, packet.num_samples * sizeof(float));

        xSemaphoreGive(lidar_mutex);
    }
    return out_packet;
}

lidar_info getLidarInfo(){
    lidar_info out;
    if(xSemaphoreTake(info_mutex, 5) == pdTRUE){
        out = info;
        xSemaphoreGive(info_mutex);
    }
    return out;
}