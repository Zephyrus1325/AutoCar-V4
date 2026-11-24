#include "navigation/chunks/chunk_task.h"
#include "chunk_arithmetic.h"

#include "common/lidar_data.h"
#include "common/robot_kinematics.h"

#include "common/configs.h"
#include "util/logger.h"

//#include "esp_dsp.h"

//using namespace dspm;

chunk_t* active_chunks[4];
chunk_t* inactive_chunk;

SemaphoreHandle_t chunk_mutex;
TaskHandle_t chunk_task_handler;

typedef struct{
    float value;    // Interpolated value
    float dx;       // Partial derivative in X direction
    float dy;       // Partial derivative in Y direction
} interp_value_t;

interp_value_t interpolatePosition(float x, float y){
    interp_value_t output;
    int32_t x0 = (int32_t)floorf(x);    // ^ Y
    int32_t y0 = (int32_t)floorf(y);    // | (x0, y1) | (x1, y1)
    int32_t x1 = x0 + 1;                // | (x0, y0) | (x1, y0)
    int32_t y1 = y0 + 1;                // +--------------------> X

    float u = x - (float)x0;
    float v = y - (float)y0;

    float P00 = (float)getGridValue(x0, y0) / 255.f; // ^ Y
    float P10 = (float)getGridValue(x1, y0) / 255.f; // | (P01) | (P11)
    float P01 = (float)getGridValue(x0, y1) / 255.f; // | (P00) | (P10)
    float P11 = (float)getGridValue(x1, y1) / 255.f; // +--------------------> X


    // Do some algebra magic... (Bilinear interpolation)
    output.value = (1.0f - u) * (1.0f - v) * P00 + u * (1.0f - v) * P10 + (1.0f - u) * v * P01 + u * v * P11;
    output.dx = (1.0f - v) * (P10 - P00) + v * (P11 - P01);
    output.dy = (1.0f - u) * (P01 - P00) + u * (P11 - P10);

    return output;
}

bool doSlam(lidar_packet* lidar, robot_kinematic* robot){

    for(int iteration = 0; iteration < CHUNK_MAX_ITERATIONS; iteration++){
        float H[3][3] = {0};
        float sigma[3] = {0};

        float sin_theta = sinf(robot->position.heading);
        float cos_theta = cosf(robot->position.heading);

        for(int i = 0; i < lidar->num_samples; i++){
            // Keep this until I have the balls to make lidar_task do this (and adapt everyone else to deal with this...)
            float sample_x = lidar->distance(i) * cosf(lidar->angle(i)); // In local lidar coordinates
            float sample_y = lidar->distance(i) * sinf(lidar->angle(i)); // In local lidar coordinates

            float world_x = (cos_theta * sample_x) - (sin_theta * sample_y) + robot->position.x;
            float world_y = (sin_theta * sample_x) + (cos_theta * sample_y) + robot->position.y;

            float grid_x = world_x * (1.f/CHUNK_PRECISION);   // Convert points from cm to chunk
            float grid_y = world_y * (1.f/CHUNK_PRECISION);   // Convert points from cm to chunk

            interp_value_t cell = interpolatePosition(grid_x, grid_y);

            float error = 1.0f - cell.value;

            float j_x = cell.dx * (1.f/CHUNK_PRECISION);
            float j_y = cell.dy * (1.f/CHUNK_PRECISION);
            

            float j_theta = cell.dx * (1.f/CHUNK_PRECISION) * (-sin_theta * sample_x - cos_theta * sample_y) +
                            cell.dy * (1.f/CHUNK_PRECISION) * ( cos_theta * sample_x - sin_theta * sample_y);

            // Acumula a Matriz Hessiana (H = J^T * J) - Aproveitando a simetria de H
            H[0][0] += j_x * j_x;     
            H[0][1] += j_x * j_y;     
            H[0][2] += j_x * j_theta;                 
            H[1][1] += j_y * j_y;     
            H[1][2] += j_y * j_theta;                                             
            H[2][2] += j_theta * j_theta;

            // Acumula o Vetor de Gradiente Residual (Sigma = J^T * erro)
            sigma[0] += j_x * error;
            sigma[1] += j_y * error;
            sigma[2] += j_theta * error;
        }

        // Preenche a metade inferior espelhada da Hessiana
        H[1][0] = H[0][1];
        H[2][0] = H[0][2];
        H[2][1] = H[1][2];

        float lambda = 0.01f; // Fator de amortecimento (mola virtual)

        H[0][0] += lambda;
        H[1][1] += lambda;
        H[2][2] += lambda;

        // Tchebychev/Regra de Cramer para resolver H * delta_pose = sigma
        // Como a matriz é minúscula (3x3), resolvemos direto via determinantes (Ultra rápido)
        float det_H = H[0][0] * (H[1][1] * H[2][2] - H[1][2] * H[2][1]) -
                      H[0][1] * (H[1][0] * H[2][2] - H[1][2] * H[2][0]) +
                      H[0][2] * (H[1][0] * H[2][1] - H[1][1] * H[2][0]);



        // Proteção contra divisão por zero (matriz singular = Lidar não encontrou casamento geométrico)
        if (fabsf(det_H) < 1e-6f) {
            //Serial.println("FUCK YOU I GIVE UP");
            return false; 
        }

        // Determinantes auxiliares corrigidos para isolar dx, dy, dtheta
        float det_dx = sigma[0] * (H[1][1] * H[2][2] - H[1][2] * H[2][1]) -
               H[0][1]  * (sigma[1] * H[2][2] - H[1][2] * sigma[2]) +
               H[0][2]  * (sigma[1] * H[2][1] - H[1][1] * sigma[2]);

        float det_dy = H[0][0] * (sigma[1] * H[2][2] - H[1][2] * sigma[2]) -
               sigma[0] * (H[1][0] * H[2][2] - H[1][2] * H[2][0]) +
               H[0][2]  * (H[1][0] * sigma[2] - sigma[1] * H[2][0]);

        // CORRIGIDO: Agora seguindo estritamente a submatriz da terceira coluna
        float det_dt = H[0][0] * (H[1][1] * sigma[2] - sigma[1] * H[2][1]) -
               H[0][1] * (H[1][0] * sigma[2] - sigma[1] * H[2][0]) +
               sigma[0] * (H[1][0] * H[2][1] - H[1][1] * H[2][0]);
        // Calcula o ajuste fino da pose
        float delta_x     = det_dx / det_H;
        float delta_y     = det_dy / det_H;
        float delta_theta = det_dt / det_H;

        //Serial.printf("dx: %7.2f | dy: %7.2f | dt: %7.2f\n\r", delta_x, delta_y, delta_theta);
        float correction_factor = 0.8f;
        // Atualiza a pose de busca
        robot->position.x += delta_x * CHUNK_PRECISION * correction_factor;
        robot->position.y += delta_y * CHUNK_PRECISION * correction_factor;
        robot->position.heading += delta_theta * correction_factor;

        // Critério de parada precoce se a correção for irrelevante (convergência)
        if (fabsf(delta_x) < CHUNK_CONVERGE_LIMIT && 
            fabsf(delta_y) < CHUNK_CONVERGE_LIMIT && 
            fabsf(delta_theta) < CHUNK_CONVERGE_LIMIT) {
            break;
        }
    }

    // Normaliza o ângulo final entre -PI e PI
    robot->position.heading = atan2f(sinf(robot->position.heading), cosf(robot->position.heading));

    return true;
}

void updateMap(lidar_packet* lidar, robot_kinematic robot){

    float sin_theta = sinf(robot.position.heading);
    float cos_theta = cosf(robot.position.heading);

    float robot_x = robot.position.x * (1.f/CHUNK_PRECISION);
    float robot_y = robot.position.y * (1.f/CHUNK_PRECISION);

    for(int i = 0; i < lidar->num_samples; i++){
        float sample_x = lidar->distance(i) * cosf(lidar->angle(i)); // In local lidar coordinates
        float sample_y = lidar->distance(i) * sinf(lidar->angle(i)); // In local lidar coordinates

        float world_x = (cos_theta * sample_x) - (sin_theta * sample_y) + robot.position.x;
        float world_y = (sin_theta * sample_x) + (cos_theta * sample_y) + robot.position.y;

        int32_t grid_x = (int32_t) (world_x * (1.f/CHUNK_PRECISION));   // Convert points from cm to chunk
        int32_t grid_y = (int32_t) (world_y * (1.f/CHUNK_PRECISION));   // Convert points from cm to chunk units

        uint16_t num_steps = (uint16_t) roundf(lidar->distance(i) * (1.f/CHUNK_PRECISION));
        if(num_steps == 0){continue;}
        // ---------- From now on, everything below is in UNITS, **NOT** cm ----------

        float dx = (grid_x - robot_x) / (float) num_steps;
        float dy = (grid_y - robot_y) / (float) num_steps;

        float x = robot_x;
        float y = robot_y;

        if(xSemaphoreTake(chunk_mutex, 5) == pdTRUE){
            for(int step = 0; step < num_steps; step++){
                addGridValue(x, y, -1);
                x += dx;
                y += dy;

            }  
            addGridValue(x, y, 2);
            xSemaphoreGive(chunk_mutex);

        } 
        
    }
}


void chunk_task(void* param){
    // First allocation
    inactive_chunk = nullptr;

    // Generate inicial chunks, for now
    // TODO: Read from Flash first of all

    if(xSemaphoreTake(chunk_mutex, 50) == pdTRUE){
        active_chunks[0] = generateChunk(0, 0, CHUNK_SIZE, CHUNK_PRECISION);
        active_chunks[1] = generateChunk(0, -1, CHUNK_SIZE, CHUNK_PRECISION);
        active_chunks[2] = generateChunk(-1, 0, CHUNK_SIZE, CHUNK_PRECISION);
        active_chunks[3] = generateChunk(-1, -1, CHUNK_SIZE, CHUNK_PRECISION);
        xSemaphoreGive(chunk_mutex);
    }

    bool chunk_malloc_failed = active_chunks[0] == nullptr || active_chunks[1] == nullptr || active_chunks[2] == nullptr || active_chunks[3] == nullptr;

    if(chunk_malloc_failed){
        print_error("Chunk Allocation Failed.");
    } else {
        print_log("Chunk allocation OK.");
    }

    bool lidar_available = false;
    subscribe_lidar(&lidar_available);
    lidar_packet* lidar_data = (lidar_packet*) malloc(sizeof(lidar_packet) + 40 * sizeof(float) * 2); 

    if(chunk_malloc_failed || lidar_data == nullptr){
        print_log("Chunk init Failed.");
    } else {
        print_log("Chunk init OK.");
    }
    

    // Create initial map without localization
    while(millis() < 10000){
        if(lidar_available){
            lidar_info info = getLidarInfo();
            if(info.type == LIDAR_NORMAL){  
                getLidarData(lidar_data);
                robot_kinematic robot = getKinematics();
                updateMap(lidar_data, robot);
             }
            lidar_available = false;
        }
        vTaskDelay(CHUNK_DELAY);
    }

    while(true){
        if(lidar_available){
            lidar_info info = getLidarInfo();
            if(info.type == LIDAR_NORMAL){  
                uint64_t start = millis();
                getLidarData(lidar_data);
                robot_kinematic robot = getKinematics();
                robot_kinematic new_robot = robot;
                if(doSlam(lidar_data, &new_robot) && 0){
                    float delta_x = (new_robot.position.x - robot.position.x);
                    float delta_y = (new_robot.position.y - robot.position.y);
                    float var = sqrtf(delta_x * delta_x + delta_y * delta_y);
                    
                    if(var > 3.f && var < 6.f){
                        float delta_h = new_robot.position.heading - robot.position.heading;
                        float delta_heading = atan2(sin(delta_h), cos(delta_h)) * (abs(robot.velocity.angular * 2) > 2.f ? 0.01f : 0.9f);
                        setPositionCorrection(delta_x * 0.04f, delta_y * 0.04f, delta_heading * 0.002f);
                    } else if(var > 1.f && var < 3.f){
                        setPositionCorrection(delta_x * 0.05f, delta_y * 0.05f, 0);
                    }
                       
                    uint64_t end = millis();
                    //Serial.printf("Parse Time: %d ms - X: %8.2f Y: %8.2f H: %8.2f\n\r", end-start, robot.position.x, robot.position.y, robot.position.heading * RAD_TO_DEG);
                    
                }
                updateMap(lidar_data, robot);
            }
            lidar_available = false;
        }

        vTaskDelay(CHUNK_DELAY);
    }
}

