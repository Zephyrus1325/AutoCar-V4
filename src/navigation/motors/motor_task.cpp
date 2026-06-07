#include <Arduino.h>
#include "navigation/motors/motor_task.h"

#include "Motor.h"

TaskHandle_t motor_task_handler;

constexpr int PIN_A = 18;
constexpr int PIN_B = 19;
constexpr int PIN_PWM = 21;
constexpr int ENC_A = 15;
constexpr int ENC_B = 2;

Motor motor(PIN_A, PIN_B, PIN_PWM, ENC_A, ENC_B);

void IRAM_ATTR update_encoder(){
    motor.readEncoder();
}

void motor_task(void* param){
    motor.begin();
    motor.setPID(0.010f, 0.09f, 0.f); // 0.0080f, 0.0020, 0
    attachInterrupt(digitalPinToInterrupt(ENC_A), update_encoder, RISING);

    motor.setRPM(70.f);

    while(true){
        //Serial.print("Values: ");
        motor.update();

        Serial.print(motor.getRPM(), 2);
        Serial.print(" ");
        Serial.print(motor.getThrottle() * 100.f, 2);
        Serial.print(" ");
        Serial.print(motor.getSetpoint(), 2);
        Serial.println(" ");

        vTaskDelay(10);
    }
}