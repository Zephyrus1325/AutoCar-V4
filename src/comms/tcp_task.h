#ifndef TELEMETRY_TASK_H
#define TELEMETRY_TASK_H

#include <Arduino.h>

extern TaskHandle_t tcp_task_handler;
void tcp_task(void* param);

#endif // TELEMETRY_TASK_H