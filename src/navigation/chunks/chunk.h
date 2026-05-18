#ifndef CHUNK_H
#define CHUNK_H

#include <Arduino.h>
#include "navigation/navigation_common.h"

typedef struct {
    int16_t x;      // in ChunkUnits
    int16_t y;      // in ChunkUnits
    uint16_t size;          // in units
    uint16_t precision;     // in mm
    uint8_t reserved[20];   // Future use
    uint8_t* data;          // Needs to be allocated
} __attribute__((packed)) chunk_t;


#endif // CHUNK_H