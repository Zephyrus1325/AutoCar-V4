#include "kinematics_task.h"
#include "common/configs.h"

#include "common/robot_kinematics.h"
#include "common/motor_data.h"
#include "common/imu_data.h"

#include "util/logger.h"

SemaphoreHandle_t kinematics_mutex;
TaskHandle_t kinematics_task_handler;

// Posição e velocidade oficiais mantidas pelo sistema
robot_kinematic kinematics = {0};

// Opiniões de ajuste vindas externamente do Lidar (parseLidar)
float kin_nudge_x = 0.0f;
float kin_nudge_y = 0.0f;
float kin_nudge_heading = 0.0f;

void kinematics_task(void* param){
    // Carrega o estado atual na inicialização
    robot_kinematic temp = {0};
    
    uint64_t last_update = millis();

    while(true){
        uint64_t act_time = millis();
        float delta_time = (float)(act_time - last_update) / 1000.f;
        last_update = act_time;
        
        motor_data motor = getMotorData();
        imu_data imu = getImuData();
        
        // 1. Coleta dados de sensores (Odometria / IMU)
        temp.velocity.linear = motor.linear_velocity;
        temp.velocity.angular = imu.gyro.z; 

        const float angular_deadband = DEG_TO_RAD * 2.0f;
        
        // Integrador clássico da Odometria (Predição)
        if(fabs(temp.velocity.angular) > angular_deadband){
            temp.position.heading += temp.velocity.angular * delta_time;
            
            // Normalização rápida do Heading entre [-PI, PI]
            if (temp.position.heading > PI)  temp.position.heading -= TWO_PI;
            if (temp.position.heading < -PI) temp.position.heading += TWO_PI;
        }
        
        temp.position.x += cos(temp.position.heading) * temp.velocity.linear * delta_time;
        temp.position.y += sin(temp.position.heading) * temp.velocity.linear * delta_time;

        // 2. Seção Crítica: Aplicação discreta do ajuste do SLAM (Correção)
        if(xSemaphoreTake(kinematics_mutex, pdMS_TO_TICKS(5)) == pdTRUE){
            
            // Consome o nudge instantâneo acumulado pelo Lidar (Se houver)
            if(fabs(kin_nudge_x) > 0.0001f || fabs(kin_nudge_y) > 0.0001f || fabs(kin_nudge_heading) > 0.0001f){
                
                // Aplica o offset diretamente na posição uma única vez
                temp.position.x += kin_nudge_x;
                temp.position.y += kin_nudge_y;
                temp.position.heading += kin_nudge_heading;
                
                // Normaliza o heading novamente após sofrer o nudge do Lidar
                if (temp.position.heading > PI)  temp.position.heading -= TWO_PI;
                if (temp.position.heading < -PI) temp.position.heading += TWO_PI;

                // Consumido com sucesso! Zeramos para o próximo ciclo do Lidar
                kin_nudge_x = 0.0f;
                kin_nudge_y = 0.0f;
                kin_nudge_heading = 0.0f;
            }

            // Publica o resultado final estruturado para o restante do firmware (ex: Navigation)
            kinematics = temp;
            xSemaphoreGive(kinematics_mutex);
        }

        //Serial.printf("X: %.2f Y: %.2f H: %.2f°\n\r", temp.position.x, temp.position.y, temp.position.heading * RAD_TO_DEG);
        
        vTaskDelay(pdMS_TO_TICKS(KINEMATICS_DELAY));
    }
}

robot_kinematic getKinematics(){
    robot_kinematic out;
    if(xSemaphoreTake(kinematics_mutex, pdMS_TO_TICKS(5)) == pdTRUE){
        out = kinematics;
        xSemaphoreGive(kinematics_mutex);
    }
    return out;
}

void setPositionCorrection(float delta_x, float delta_y, float delta_heading){
    if(xSemaphoreTake(kinematics_mutex, 5) == pdTRUE){
        kin_nudge_x = delta_x;
        kin_nudge_y = delta_x;
        kin_nudge_heading = delta_heading;
        xSemaphoreGive(kinematics_mutex);
    }
}

void setPosition(float x, float y, float heading){
    if(xSemaphoreTake(kinematics_mutex, 5) == pdTRUE){
        kinematics.position.x;
        kinematics.position.y;
        kinematics.position.heading;
        xSemaphoreGive(kinematics_mutex);
    }
}