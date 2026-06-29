#ifndef HEALTH_TASK_H
#define HEALTH_TASK_H

extern TaskHandle_t imu_task_handler;
void health_task(void* param);

#endif // HEALTH_TASK_H