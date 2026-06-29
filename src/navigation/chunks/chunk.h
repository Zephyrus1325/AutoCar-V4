#ifndef CHUNK_H
#define CHUNK_H

#include <Arduino.h>

extern SemaphoreHandle_t chunk_mutex;

typedef struct {
    int32_t x;
    int32_t y;
} vector_t;

typedef struct {
    int16_t x;      // in ChunkUnits
    int16_t y;      // in ChunkUnits
    uint16_t size;          // in units
    uint16_t precision;     // in mm
    uint8_t map_id;         // Id of actual location
    uint8_t reserved[20];   // Future use
    uint8_t data[];          // Needs to be allocated
} __attribute__((packed)) chunk_t;

// Remember - All of those are **POINTERS** to chunks
// Get fucking read to do a LOT of pointers interchange, hope to not lose them!!
extern chunk_t* active_chunks[4]; // Chunks being actually used by the System  
extern chunk_t* inactive_chunk;   // Chunk being loaded/stored (AKA, unreliable or sensible to changes)

#endif // CHUNK_H