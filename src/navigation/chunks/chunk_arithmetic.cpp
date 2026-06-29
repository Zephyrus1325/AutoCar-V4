#include "chunk_arithmetic.h"
#include "common/configs.h"

// return global position in mm
vector_t chunkLocalToGlobalCoordinates(uint16_t x, uint16_t y, int16_t chunk_x, int16_t chunk_y){
    vector_t position;
    //         |   Gross position   |   Finer position
    position.x = (chunk_x * CHUNK_SIZE + x) * CHUNK_PRECISION / 10.f;
    position.y = (chunk_y * CHUNK_SIZE + y) * CHUNK_PRECISION / 10.f;

    return position;
}

// Global -> ChunkPos
vector_t globalToChunkLocalCoordinates(int32_t y, int32_t x){
    vector_t position;

    position.x = (x * 10 / CHUNK_PRECISION) % (uint32_t)(CHUNK_SIZE);
    position.y = (y * 10 / CHUNK_PRECISION) % (uint32_t)(CHUNK_SIZE);

    return position;
}

// Global -> Chunk
vector_t globalToChunkCoordinates(int32_t x, int32_t y){
    vector_t position;

    position.x = x / (int32_t)(CHUNK_SIZE * CHUNK_PRECISION / 10.f);
    position.y = y / (int32_t)(CHUNK_SIZE * CHUNK_PRECISION / 10.f);

    return position;
}

 // Its in local coordinates
void setChunkValue(uint16_t x, uint16_t y, uint16_t value, chunk_t* chunk){
    uint16_t address = y * CHUNK_SIZE + x;
    chunk->data[address] = value;
}

// // Its in local coordinates
//uint16_t getChunkValue(uint16_t x, uint16_t y, chunk_t* chunk){
//    uint16_t value_out;
//    uint16_t address = y * CHUNK_SIZE + x;
//    value_out = chunk->data[address];
//}