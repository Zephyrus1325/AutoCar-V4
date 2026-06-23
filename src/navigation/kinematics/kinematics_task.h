#ifndef KINEMATICS_TASK_H
#define KINEMATICS_TASK_H

#include <Arduino.h>

extern TaskHandle_t kinematics_task_handler;
void kinematics_task(void* param);

#endif // KINEMATICS_TASK_H