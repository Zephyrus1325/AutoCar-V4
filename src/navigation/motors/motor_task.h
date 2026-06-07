#ifndef MOTOR_TASK_H
#define MOTOR_TASK_H

#include <Arduino.h>
#include "Motor.h"

extern Motor motor;
extern TaskHandle_t motor_task_handler;
void motor_task(void* param);

#endif // MOTOR_TASK_H