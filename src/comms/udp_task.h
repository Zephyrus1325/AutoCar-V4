#ifndef CHUNK_TASK_H
#define CHUNK_TASK_H

#include <Arduino.h>

extern TaskHandle_t udp_task_handler;
void udp_task(void* param);

#endif // CHUNK_TASK_H