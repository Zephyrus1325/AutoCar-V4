#ifndef NETWORKER_H
#define NETWORKER_H

#include <Arduino.h>

extern TaskHandle_t network_handler;
void network_task(void* param);


#endif // NETWORKER_H