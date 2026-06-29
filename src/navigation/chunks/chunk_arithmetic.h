#ifndef CHUNK_MATH_H
#define CHUNK_MATH_H

#include "chunk.h"

vector_t chunkLocalToGlobalCoordinates(uint16_t x, uint16_t y); // return global position in mm

vector_t globalToChunkLocalCoordinates(int32_t x, int32_t y);   // Global -> ChunkPos

vector_t globalToChunkCoordinates(int32_t x, int32_t y);    // Global -> Chunk

void setChunkValue(int32_t x, int32_t y, uint16_t value, chunk_t* chunk); // Its in local coordinates

//uint16_t getChunkValue(int32_t x, int32_t y, chunk_t* chunk); // Its in local coordinates
#endif // CHUNK_MATH_H