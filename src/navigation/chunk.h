#ifndef CHUNK_H
#define CHUNK_H

#include "Arduino.h"
#include "common/configs.h"

typedef struct {
    int16_t x;
    int16_t y;
    uint16_t size;
    uint16_t precision;
    uint8_t reserved[20];
    uint8_t* data;          // Needs to be allocated
} __attribute__((packed)) chunk_t;


#endif // CHUNK_H