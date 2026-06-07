#ifndef IMU_TASK_H
#define IMU_TASK_H

#include <Arduino.h>
#include "MPU6050.h"

extern MPU6050 mpu;
extern TaskHandle_t imu_task_handler;
void imu_task(void* param);


#endif // IMU_TASK_H