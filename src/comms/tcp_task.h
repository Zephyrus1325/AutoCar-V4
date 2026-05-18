#ifndef TCP_TASK_H
#define TCP_TASK_H

#include <Arduino.h>

extern TaskHandle_t tcp_task_handler;
void tcp_task(void* param);

#endif // TCP_TASK_H