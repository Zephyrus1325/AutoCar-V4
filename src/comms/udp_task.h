#ifndef UDP_TASK_H
#define UDP_TASK_H

#include <Arduino.h>

extern TaskHandle_t udp_task_handler;
void udp_task(void* param);

#endif // UDP_TASK_H