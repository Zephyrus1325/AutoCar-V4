#ifndef QUEUES_H
#define QUEUES_H

#include <Arduino.h>

QueueHandle_t lidar_readings_network;       // Lidar -> Network
QueueHandle_t lidar_readings_navigation;    // Lidar -> Navigation

#endif // QUEUES_H