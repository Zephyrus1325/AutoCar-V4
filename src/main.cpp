#include <Arduino.h>

#include "common/configs.h"

#include "comms/network_task.h"
#include "sensors/lidar/lidar_task.h"
//#include "navigation/chunks/chunk_task.h"
//#include "sensors/IMU/imu_task.h"
#include "navigation/motors/motor_task.h"
#include "util/file_read.h"
#include "util/logger.h"

void setup() {
    Serial.begin(115200);   // Starts Serial console for ease of debug

    setupFileSystem();

    analogWriteFrequency(25000);
    analogWriteResolution(10);

    xTaskCreatePinnedToCore(network_task, "Network Task", NETWORK_STACK_SIZE, NULL, 1, &network_handler, NETWORK_CORE);
    xTaskCreatePinnedToCore(lidar_task, "Lidar Task", LIDAR_STACK_SIZE, NULL, 1, &lidar_task_handler, LIDAR_CORE);
    //xTaskCreatePinnedToCore(chunk_task, "Chunk Task", CHUNK_STACK_SIZE, NULL, 1, &chunk_task_handler, CHUNK_CORE);
    //xTaskCreatePinnedToCore(imu_task, "IMU Task", IMU_STACK_SIZE, NULL, 1, &imu_task_handler, IMU_CORE);
    xTaskCreatePinnedToCore(motor_task, "Motor Task", MOTOR_STACK_SIZE, NULL, 1, &motor_task_handler, MOTOR_CORE);

}



void loop() {
    vTaskDelay(portMAX_DELAY);  // Basically disables this task
}