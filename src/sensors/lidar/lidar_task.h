#ifndef LIDAR_TASK_H
#define LIDAR_TASK_H

#include <Arduino.h>
#include "Lidar.h"

extern Lidar lidar;
extern TaskHandle_t lidar_task_handler;
void lidar_task(void* param);


#endif // LIDAR_TASK_H