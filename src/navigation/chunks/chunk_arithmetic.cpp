#include "chunk_arithmetic.h"
#include "common/configs.h"

#include "util/logger.h"

// TODO: Make it so it doesnt allocate memory, let the caller allocate it for us
chunk_t* generateChunk(int16_t x, int16_t y, uint16_t size, uint16_t precision){
    uint32_t total_size = sizeof(chunk_t) + (sizeof(uint8_t) * size * size);
    
    chunk_t* c = (chunk_t*) ps_malloc(total_size);

    // PROTEÇÃO ABSOLUTA: Se o ESP32 estiver sem RAM, evita o crash
    if (c == nullptr) {
        print_error("Failure during chunk allocation");
        return nullptr; 
    }
    
    c->x = x;
    c->y = y;
    c->size = size;
    c->precision = precision;
    c->map_id = 0;      

    // Limpa os valores de forma segura
    for(int i = 0; i < size * size; i++){
        c->data[i] = 127; 
    }

    return c; // Retorna o ponteiro correto e funcional
}

void deleteChunk(chunk_t* c){
    if(c == nullptr){return;}
    c->size = 0;
    c->precision = 0;
    free(c);
}

