#include <Arduino.h>
#include "common/configs.h"

#include "navigation/motors/motor_task.h"
#include "util/file_read.h"
#include "util/logger.h"

#include "Motor.h"
#include "common/motor_data.h"

SemaphoreHandle_t motor_mutex;
TaskHandle_t motor_task_handler;

Motor left_motor(MOTOR_LEFT_A_PIN, MOTOR_LEFT_B_PIN, MOTOR_LEFT_PWM_PIN, MOTOR_LEFT_ENC_A_PIN, MOTOR_LEFT_ENC_B_PIN);
Motor right_motor(MOTOR_RIGHT_A_PIN, MOTOR_RIGHT_B_PIN, MOTOR_RIGHT_PWM_PIN, MOTOR_RIGHT_ENC_A_PIN, MOTOR_RIGHT_ENC_B_PIN);

void IRAM_ATTR update_left_encoder(){
    left_motor.readEncoder();
}

void IRAM_ATTR update_right_encoder(){
    right_motor.readEncoder();
}

motor_data data;

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

    float sum_angvel = 0;
    float last_angvel[5] = {0};
    uint16_t angvel_index = 0;
    uint64_t last_time = millis();

    motor_data temp;

    while(true){
        // Update motors PIDs
        left_motor.update();
        right_motor.update();

        // Filling basic motor info
        temp.left.rpm = left_motor.getRPM();
        temp.left.setpoint = left_motor.getSetpoint();
        temp.left.throttle = left_motor.getThrottle();
        
        temp.right.rpm = right_motor.getRPM();
        temp.right.setpoint = right_motor.getSetpoint();
        temp.right.throttle = right_motor.getThrottle();

        // Independend linear velocity Calculation
        temp.left.linear_velocity = left_motor.getRPM() * PI * WHEEL_DIAMETER / 60.f;
        temp.right.linear_velocity = right_motor.getRPM() * PI * WHEEL_DIAMETER / 60.f;

        // Composite velocities calculation
        temp.linear_velocity = (temp.left.linear_velocity + temp.right.linear_velocity) / 2.f;
        float angular_vel = (temp.left.linear_velocity - temp.right.linear_velocity) / WHEEL_BASE;

        // Circular buffer update
        sum_angvel -= last_angvel[angvel_index];
        sum_angvel += angular_vel;
        last_angvel[angvel_index] = angular_vel;
        if (++angvel_index >= 5) {angvel_index = 0; }

        temp.angular_velocity = sum_angvel / 5.f;  // Get filtered out velocity

        // Add it to official reading
        if(xSemaphoreTake(motor_mutex, 5) == pdTRUE){
            data = temp;
            xSemaphoreGive(motor_mutex);
        }
        
        vTaskDelay(MOTOR_DELAY);
    }
}

void setMotorSpeed(float left, float right){
    left_motor.setRPM(left /(PI * WHEEL_DIAMETER / 60.f) );
    right_motor.setRPM(right /(PI * WHEEL_DIAMETER / 60.f) );
}

motor_data getMotorData(){
    motor_data out;

    if(xSemaphoreTake(motor_mutex, 5) == pdTRUE){
        out = data;
        xSemaphoreGive(motor_mutex);
    }

    return out;
}