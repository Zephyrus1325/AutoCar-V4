#ifndef NAVIGATION_TASK_H
#define NAVIGATION_TASK_H

#include <Arduino.h>

extern TaskHandle_t navigation_task_handler;
void navigation_task(void* param);

#endif // NAVIGATION_TASK_H