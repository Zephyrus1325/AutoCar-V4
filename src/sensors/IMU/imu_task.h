#ifndef IMU_TASK_H
#define IMU_TASK_H

#include <Arduino.h>

extern TaskHandle_t imu_task_handler;
void imu_task(void* param);
extern float imu_heading;

#endif // IMU_TASK_H