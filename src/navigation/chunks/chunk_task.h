#ifndef CHUNK_TASK_H
#define CHUNK_TASK_H

#include <Arduino.h>
#include "navigation/chunks/chunk.h"

// Remember - All of those are **POINTERS** to chunks
// Get fucking read to do a LOT of pointers interchange, hope to not lose them!!
extern chunk_t* active_chunks[4]; // Chunks being actually used by the System  
extern chunk_t* inactive_chunk;   // Chunk being loaded/stored (AKA, unreliable or sensible to changes)

extern TaskHandle_t chunk_task_handler;
void chunk_task(void* param);

#endif // CHUNK_TASK_H