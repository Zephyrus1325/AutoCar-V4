#include <Arduino.h>

#include "common/configs.h"

#include "comms/network_task.h"
#include "sensors/lidar/lidar_task.h"
#include "navigation/chunks/chunk_task.h"

#include "Lidar.h"

void setup() {
    Serial.begin(115200);   // Starts Serial console for ease of debug

    // Sets up pins for everyone
    pinMode(MOTOR_LEFT_PWM_PIN, OUTPUT);    
    pinMode(MOTOR_RIGHT_PWM_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_A_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_B_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_A_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_B_PIN, OUTPUT);  

    analogWriteFrequency(25000);
    analogWrite(MOTOR_LEFT_PWM_PIN, 180);
    analogWrite(MOTOR_RIGHT_PWM_PIN, 180);


    xTaskCreatePinnedToCore(network_task, "Network Task", NETWORK_STACK_SIZE, NULL, 1, &network_handler, NETWORK_CORE);
    xTaskCreatePinnedToCore(lidar_task, "Lidar Task", LIDAR_STACK_SIZE, NULL, 1, &lidar_task_handler, LIDAR_CORE);
    xTaskCreatePinnedToCore(chunk_task, "Chunk Task", CHUNK_STACK_SIZE, NULL, 1, &chunk_task_handler, CHUNK_CORE);
}



void loop() {
    vTaskDelay(portMAX_DELAY);  // Basically disables this task
}