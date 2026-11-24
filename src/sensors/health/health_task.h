#ifndef HEALTH_TASK_H
#define HEALTH_TASK_H

#include <Arduino.h>

extern TaskHandle_t health_task_handler;
void health_task(void* param);

#endif // HEALTH_TASK_H