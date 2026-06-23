#include "navigation/chunks/chunk_task.h"
#include "common/configs.h"
#include "util/logger.h"

chunk_t* active_chunks[4];
chunk_t* inactive_chunk;

TaskHandle_t chunk_task_handler;

chunk_t* generateChunk(int16_t x, int16_t y, uint16_t size, uint16_t precision){
    uint32_t total_size = sizeof(chunk_t) + (sizeof(uint8_t) * size * size);
    
    chunk_t* c = (chunk_t*) ps_malloc(total_size);
    
    // PROTEÇÃO ABSOLUTA: Se o ESP32 estiver sem RAM, evita o crash
    if (c == nullptr) {
        print_error("Failure during chunk allocation");
        return nullptr; 
    }
    
    c->type = 2;    
    c->x = x;
    c->y = y;
    c->size = size;
    c->precision = precision;
    c->map_id = 0;      
    
    // CORREÇÃO DA ARITMÉTICA: Avança exatamente os bytes da struct usando cast para uint8_t*
    c->data = (uint8_t*)(c) + sizeof(chunk_t);

    // Limpa os valores de forma segura
    for(int i = 0; i < size * size; i++){
        c->data[i] = 255 * (i & 0x1); 
    }

    return c; // Retorna o ponteiro correto e funcional
}

void deleteChunk(chunk_t* c){
    c->size = 0;
    c->precision = 0;
    free(c->data);
}

void chunk_task(void* param){
    // First allocation
    inactive_chunk = nullptr;

    // Generate inicial chunks, for now
    // TODO: Read from SD card first of all
    active_chunks[0] = generateChunk(0, 0, CHUNK_SIZE, CHUNK_PRECISION);
    active_chunks[1] = generateChunk(0, -1, CHUNK_SIZE, CHUNK_PRECISION);
    active_chunks[2] = generateChunk(-1, 0, CHUNK_SIZE, CHUNK_PRECISION);
    active_chunks[3] = generateChunk(-1, -1, CHUNK_SIZE, CHUNK_PRECISION);

    while(true){
        vTaskDelay(portMAX_DELAY);
    }
}