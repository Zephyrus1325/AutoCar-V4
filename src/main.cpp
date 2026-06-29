#include <Arduino.h>
#include "common/configs.h"

#include "Wire.h"

#include "comms/network_task.h"
#include "sensors/lidar/lidar_task.h"
#include "navigation/chunks/chunk_task.h"
#include "navigation/kinematics/kinematics_task.h"
#include "sensors/IMU/imu_task.h"
#include "navigation/motors/motor_task.h"

#include "util/file_read.h"
#include "util/logger.h"

extern SemaphoreHandle_t lidar_mutex;
extern SemaphoreHandle_t info_mutex;
extern SemaphoreHandle_t imu_mutex;
extern SemaphoreHandle_t motor_mutex;
extern SemaphoreHandle_t kinematics_mutex;

void setup() {
    Serial.begin(115200);   // Starts Serial console for ease of debug
    

    lidar_mutex = xSemaphoreCreateMutex();
    info_mutex = xSemaphoreCreateMutex();
    imu_mutex = xSemaphoreCreateMutex();
    motor_mutex = xSemaphoreCreateMutex();
    kinematics_mutex = xSemaphoreCreateMutex();
    chunk_mutex = xSemaphoreCreateMutex();

    if(!psramFound()){
        print_error("PSRAM not available for some reason - Check your wiring.");
    }

    if(!psramInit()){
        print_error("PSRAM not initialized properly.");
    }

    if(!Wire.begin()){
        print_error("I2C not initialized properly.");
    }

    Wire.setClock(400000); 

    setupFileSystem();

    analogWriteFrequency(25000);
    analogWriteResolution(10);

    xTaskCreatePinnedToCore(network_task, "Network Task", NETWORK_STACK_SIZE, NULL, NETWORK_PRIORITY, &network_handler, NETWORK_CORE);
    xTaskCreatePinnedToCore(lidar_task, "Lidar Task", LIDAR_STACK_SIZE, NULL, LIDAR_PRIORITY, &lidar_task_handler, LIDAR_CORE);
    xTaskCreatePinnedToCore(chunk_task, "Chunk Task", CHUNK_STACK_SIZE, NULL, CHUNK_PRIORITY, &chunk_task_handler, CHUNK_CORE);
    xTaskCreatePinnedToCore(imu_task, "IMU Task", IMU_STACK_SIZE, NULL, IMU_PRIORITY, &imu_task_handler, IMU_CORE);
    xTaskCreatePinnedToCore(motor_task, "Motor Task", MOTOR_STACK_SIZE, NULL, MOTOR_PRIORITY, &motor_task_handler, MOTOR_CORE);
    xTaskCreatePinnedToCore(kinematics_task, "Kinematics Task", KINEMATICS_STACK_SIZE, NULL, KINEMATICS_PRIORITY, &kinematics_task_handler, KINEMATICS_CORE);
}

void loop() {
    vTaskDelay(portMAX_DELAY);  // Basically disables this task
}