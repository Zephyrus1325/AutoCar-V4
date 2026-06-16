#include "navigation/chunks/chunk_task.h"
#include "common/configs.h"


chunk_t* active_chunks[4];
chunk_t* inactive_chunk;

TaskHandle_t chunk_task_handler;

void generateChunk(chunk_t* c, int16_t x, int16_t y, uint16_t size, uint16_t precision){
    c->x = x;
    c->y = y;
    c->size = size;
    c->precision = precision;
    c->map_id = 0;      // Well, try to get a way to set this correctlyl0
    c->data = (uint8_t*) malloc(size * size);    // Allocate the memory for that chunk

    // Clear chunk values
    for(int i = 0; i < size*size; i++){
        c->data[i] = 0;
    }
}

void deleteChunk(chunk_t* c){
    c->size = 0;
    c->precision = 0;
    free(c->data);
}

void chunk_task(void* param){
    
    // First allocation
    active_chunks[0] = (chunk_t*) malloc(sizeof(chunk_t));
    active_chunks[1] = (chunk_t*) malloc(sizeof(chunk_t));
    active_chunks[2] = (chunk_t*) malloc(sizeof(chunk_t));
    active_chunks[3] = (chunk_t*) malloc(sizeof(chunk_t));

    inactive_chunk = (chunk_t*) malloc(sizeof(chunk_t));
    

    // Generate inicial chunks, for now
    // TODO: Read from SD card first of all
    generateChunk(active_chunks[0], 0, 0, CHUNK_SIZE, CHUNK_PRECISION);
    generateChunk(active_chunks[1], 0, -1, CHUNK_SIZE, CHUNK_PRECISION);
    generateChunk(active_chunks[2], -1, 0, CHUNK_SIZE, CHUNK_PRECISION);
    generateChunk(active_chunks[3], -1, -1, CHUNK_SIZE, CHUNK_PRECISION);

    while(true){
        vTaskDelay(portMAX_DELAY);
    }
}