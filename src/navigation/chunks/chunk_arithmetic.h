#ifndef CHUNK_MATH_H
#define CHUNK_MATH_H

#include "chunk.h"
#include "common/configs.h"

extern SemaphoreHandle_t chunk_mutex;

chunk_t* generateChunk(int16_t x, int16_t y, uint16_t size, uint16_t precision); 
void deleteChunk(chunk_t* c);

// Global -> Chunk (Agora blindado usando int32_t puro de ponta a ponta)
inline vector_t globalToChunkCoordinates(int32_t x, int32_t y){
    vector_t position;
    position.x = (x / CHUNK_SIZE) - (x < 0);
    position.y = (y / CHUNK_SIZE) - (y < 0);
    return position;
}

// Retorna o endereço indexado interno (0 a CHUNK_SIZE^2 - 1)
inline uint16_t globalToChunkAddress(int32_t x, int32_t y){
    // Garante que o indexador local seja estritamente positivo (0 a 127) mesmo se global for negativo
    uint32_t local_x = (uint32_t)(x & CHUNK_MASK);
    uint32_t local_y = (uint32_t)(y & CHUNK_MASK);
    return constrain((uint16_t)(local_y << CHUNK_BITS) + local_x, 0, CHUNK_SIZE * CHUNK_SIZE - 1);
}

// Alterado de uint16_t para int32_t para aceitar coordenadas negativas do robô!
inline void setGridValue(int32_t x, int32_t y, uint8_t data){
    vector_t chunk = globalToChunkCoordinates(x, y);
    uint16_t local_address = globalToChunkAddress(x, y);

    for(int i = 0; i < sizeof(active_chunks)/sizeof(chunk_t*); i++) {
        if(active_chunks[i] != nullptr && 
           active_chunks[i]->x == chunk.x && 
           active_chunks[i]->y == chunk.y) {
            active_chunks[i]->data[local_address] = data;
            break; // Encontrou, pode parar o laço
        }
    }
}

// Alterado de uint16_t para int32_t para aceitar coordenadas negativas do robô!
inline void addGridValue(int32_t x, int32_t y, int16_t delta){
    vector_t chunk = globalToChunkCoordinates(x, y);
    uint16_t local_address = globalToChunkAddress(x, y);

    for(int i = 0; i < sizeof(active_chunks)/sizeof(chunk_t*); i++) {
        if(active_chunks[i] != nullptr && 
           active_chunks[i]->x == chunk.x && 
           active_chunks[i]->y == chunk.y){
                active_chunks[i]->data[local_address] = constrain(active_chunks[i]->data[local_address] + delta, 0, 255);
                break;
        }
    }
}

inline uint8_t getGridValue(int32_t x, int32_t y){
    vector_t chunk = globalToChunkCoordinates(x, y);
    uint16_t local_address = globalToChunkAddress(x, y);

    for(int i = 0; i < sizeof(active_chunks)/sizeof(chunk_t*); i++) {
        if(active_chunks[i] != nullptr && 
           active_chunks[i]->x == chunk.x && 
           active_chunks[i]->y == chunk.y) {
            return active_chunks[i]->data[local_address];
        }
    }
    return 127;
}

#endif // CHUNK_MATH_H