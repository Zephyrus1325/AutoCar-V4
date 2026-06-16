#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <Arduino.h>
#include <WiFi.h>

#include <AsyncWebSocket.h>

extern AsyncWebSocket ws;
extern TaskHandle_t telemetry_handler;
void startServer();
void telemetry_task(void *args);

#endif // NETWORK_SERVER_H