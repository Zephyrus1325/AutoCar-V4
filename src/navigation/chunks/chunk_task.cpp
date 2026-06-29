#include "navigation/chunks/chunk_task.h"
#include "chunk_arithmetic.h"

#include "common/lidar_data.h"
#include "common/robot_kinematics.h"

#include "common/configs.h"
#include "util/logger.h"

chunk_t* active_chunks[4];
chunk_t* inactive_chunk;

SemaphoreHandle_t chunk_mutex;
TaskHandle_t chunk_task_handler;

// Bonjour, this is probably the HEAVIEST function in the project
// Like, look how many for() loops there are
void parseLidar(lidar_packet* sample){
    
    robot_kinematic robot = getKinematics();

    // Note for self - Eventually raycast the reading, and filter out the relevant chunks for that one reading, may be more efficient
    // Step 1 - Find the chunk id for actual robot position
    vector_t chunk_pos = globalToChunkCoordinates(robot.position.x, -robot.position.y);
    int start_id = -1;

    if(xSemaphoreTake(chunk_mutex, 5) == pdTRUE){
        for(int i = 0; i < 4; i++){
            if(active_chunks[i] != nullptr &&
               active_chunks[i]->x == chunk_pos.x &&
               active_chunks[i]->y == chunk_pos.y){
                start_id = i;
                break;
            }
        }
        xSemaphoreGive(chunk_mutex);
    }

    if(start_id < 0){return;}   // Gets out if theres no valid chunk with this coordinate, for some reason

    // Get chunk local coordinates
    vector_t start_pos = globalToChunkLocalCoordinates(robot.position.x, robot.position.y);     // TEST DEBUG - OK

    for(int i = 0; i < sample->num_samples; i++){
        if(sample->data[sample->num_samples + i] > CHUNK_SIZE * CHUNK_PRECISION / 20){continue;}    // Limits samples to only within loaded chunks, theoretically
        
        const float step_size = ((CHUNK_PRECISION)/10.f);       

        uint16_t num_steps = (uint16_t)(sample->data[sample->num_samples + i] / step_size);         // Get total steps

        float sample_angle = robot.position.heading + sample->data[i];   // Get sample angle

        float step_x = cos(sample_angle) * step_size / (CHUNK_PRECISION/10);         // Get step size in x direction
        float step_y = sin(sample_angle) * step_size / (CHUNK_PRECISION/10);         // Get step size in y direction

        // Set the start position
        float pos_x = start_pos.x;
        float pos_y = start_pos.y;  

        vector_t act_chunk_pos = chunk_pos; // Set the inicial chunk_pos
        int act_id = start_id;          // Set the start chunk id

        // Iterate on the steps
        for(int step = 0; step < num_steps; step++){
            const int32_t x_index = (int32_t)pos_x;
            const int32_t y_index = (int32_t)pos_y;
            uint16_t address = 0;
            int16_t value = 127;

            if(x_index >= 0 && x_index < CHUNK_SIZE && y_index >= 0 && y_index < CHUNK_SIZE){
                address = (uint16_t)y_index * CHUNK_SIZE + (uint16_t)x_index;
            } else {continue;}

            if(act_id >= 0 && act_id < 4 && xSemaphoreTake(chunk_mutex, 5) == pdTRUE){
                chunk_t* current_chunk = active_chunks[act_id];
                if(current_chunk != nullptr && current_chunk->data != nullptr && address < CHUNK_SIZE * CHUNK_SIZE){
                    value = current_chunk->data[address];
                }
                xSemaphoreGive(chunk_mutex);
            }

            value = step < (num_steps-1) ? --value : ++value;   // Remove 1 from the samples (except the last one)
            value = constrain(value, 0, 255);

            if(act_id >= 0 && act_id < 4 && xSemaphoreTake(chunk_mutex, 5) == pdTRUE){
                chunk_t* current_chunk = active_chunks[act_id];
                if(current_chunk != nullptr && current_chunk->data != nullptr && address < CHUNK_SIZE * CHUNK_SIZE){
                    current_chunk->data[address] = (uint8_t)value;
                }
                xSemaphoreGive(chunk_mutex);
            }

            // Iterate to next step
            pos_x += step_x;
            pos_y += step_y;

            // Check if has crossed chunk boundary
            bool update_chunk = false;
            if(pos_x >= CHUNK_SIZE)  {pos_x = 0;             update_chunk = true; act_chunk_pos.x++;}
            else if(pos_x < 0)      {pos_x = CHUNK_SIZE-1;  update_chunk = true; act_chunk_pos.x--;}
            if(pos_y >= CHUNK_SIZE)  {pos_y = 0;             update_chunk = true; act_chunk_pos.y++;}
            else if(pos_y < 0)      {pos_y = CHUNK_SIZE-1;  update_chunk = true; act_chunk_pos.y--;}

            if(update_chunk){
                bool found_chunk = false;
                if(xSemaphoreTake(chunk_mutex, 5) == pdTRUE){
                    for(int i = 0; i < 4; i++){
                        if(active_chunks[i] != nullptr &&
                           active_chunks[i]->x == act_chunk_pos.x &&
                           active_chunks[i]->y == act_chunk_pos.y){
                            act_id = i;
                            found_chunk = true;
                            break;
                        }
                    }
                    xSemaphoreGive(chunk_mutex);
                    if(!found_chunk){
                        //print_log("Not chunk found - Expectative: " + String( act_chunk_pos.x) + " | " + String(act_chunk_pos.y));
                        break;
                    }
                    
                }
            }
        }

    }

}



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

void chunk_task(void* param){
    // First allocation
    inactive_chunk = nullptr;

    // Generate inicial chunks, for now
    // TODO: Read from SD card first of all

    if(xSemaphoreTake(chunk_mutex, 5) == pdTRUE){
        active_chunks[0] = generateChunk(0, 0, CHUNK_SIZE, CHUNK_PRECISION);
        active_chunks[1] = generateChunk(0, -1, CHUNK_SIZE, CHUNK_PRECISION);
        active_chunks[2] = generateChunk(-1, 0, CHUNK_SIZE, CHUNK_PRECISION);
        active_chunks[3] = generateChunk(-1, -1, CHUNK_SIZE, CHUNK_PRECISION);
        xSemaphoreGive(chunk_mutex);
    }


    if(active_chunks[0] == nullptr || active_chunks[1] == nullptr || active_chunks[2] == nullptr || active_chunks[3] == nullptr){
        print_error("Chunk Allocation Failed.");
    } else {
        print_log("Chunk allocation OK.");
    }

    bool lidar_available = false;
    subscribe_lidar(&lidar_available);
    lidar_packet* lidar_data = (lidar_packet*) malloc(sizeof(lidar_packet) + 40 * sizeof(float) * 2); 


    while(true){
        if(lidar_available){
            lidar_info info = getLidarInfo();
            if(info.type == LIDAR_NORMAL){
                getLidarData(lidar_data);
                parseLidar(lidar_data);
            }
            lidar_available = false;
        }

        vTaskDelay(CHUNK_DELAY);
    }
}