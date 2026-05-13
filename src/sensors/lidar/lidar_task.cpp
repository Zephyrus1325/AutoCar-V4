#include "sensors/lidar/lidar_task.h"
#include "common/configs.h"

#include "Lidar.h"


TaskHandle_t lidar_task_handler;
Lidar lidar;

void lidar_task(void* param){
    lidar.begin(&Serial2, LIDAR_MOTOR_PIN);
    LidarData data;
    uint32_t last_id = 0;

    lidar.setRPM(280);

    while(true){
        lidar.update();         // check if there are readings
        data = lidar.read();    // Read what there is to read

        // If there is a new, different packet
        if(last_id != data.id()){

            // If its a packet with valid measurements
            if(data.length() > 0){
                //
                // TODO: send the data to chunk manager queue;
                //
            }   

            last_id = data.id();    // Stores new last id
        }
        delay(10);         // Lidar is quite slow, so we can check for new data with slower frequency
    }
}