#ifndef CHUNK_TASK_H
#define CHUNK_TASK_H

#include <Arduino.h>
#include "navigation/chunks/chunk.h"

extern TaskHandle_t chunk_task_handler;
void chunk_task(void* param);

#endif // CHUNK_TASK_H