#include <Arduino.h>
#include "common/configs.h"

#include "navigation/motors/motor_task.h"
#include "util/file_read.h"
#include "util/logger.h"

#include "Motor.h"

TaskHandle_t motor_task_handler;

Motor left_motor(MOTOR_LEFT_A_PIN, MOTOR_LEFT_B_PIN, MOTOR_LEFT_PWM_PIN, MOTOR_LEFT_ENC_A_PIN, MOTOR_LEFT_ENC_B_PIN);
Motor right_motor(MOTOR_RIGHT_A_PIN, MOTOR_RIGHT_B_PIN, MOTOR_RIGHT_PWM_PIN, MOTOR_RIGHT_ENC_A_PIN, MOTOR_RIGHT_ENC_B_PIN);

void IRAM_ATTR update_left_encoder(){
    left_motor.readEncoder();
}

void IRAM_ATTR update_right_encoder(){
    right_motor.readEncoder();
}

void motor_task(void* param){
    left_motor.begin();
    right_motor.begin();

    JsonDocument config = getConfigData();

    float left_kp = config["motor"]["left"]["kp"];
    float left_ki = config["motor"]["left"]["ki"];
    float left_kd = config["motor"]["left"]["kd"];
    float right_kp = config["motor"]["right"]["kp"];
    float right_ki = config["motor"]["right"]["ki"];
    float right_kd = config["motor"]["right"]["kd"];

    left_motor.setPID(left_kp, left_ki, left_kd);
    right_motor.setPID(right_kp, right_ki, right_kd);
    
    attachInterrupt(digitalPinToInterrupt(MOTOR_LEFT_ENC_A_PIN), update_left_encoder, RISING);
    attachInterrupt(digitalPinToInterrupt(MOTOR_RIGHT_ENC_A_PIN), update_right_encoder, RISING);

    left_motor.setRPM(0);
    right_motor.setRPM(0);

    while(true){
        left_motor.update();
        right_motor.update();
        vTaskDelay(10);
    }
}