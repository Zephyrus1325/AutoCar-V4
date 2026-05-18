#ifndef CHUNK_MATH_H
#define CHUNK_MATH_H

#include "chunk.h"

// return global position in mm
vector_t chunkLocalToGlobalCoordinates(uint16_t x, uint16_t y, chunk_t* c){
    vector_t position;
    //         |   Gross position   |   Finer position
    position.x = (c->x * c->size + x) * c->precision;
    position.y = (c->y * c->size + y) * c->precision;

    return position;
}

// Global -> ChunkPos
vector_t globalToChunkLocalCoordinates(int32_t x, int32_t y, chunk_t* c){
    vector_t position;

    position.x = x % (c->size * c->precision);
    position.y = y % (c->size * c->precision);

    return position;
}

vector_t globalToChunkCoordinates(int32_t x, int32_t y, chunk_t* c){
    vector_t position;

    position.x = x / (c->size * c->precision);
    position.y = y / (c->size * c->precision);

    return position;
}

#endif // CHUNK_MATH_H