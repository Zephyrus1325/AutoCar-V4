#include "kinematics_task.h"
#include "common/configs.h"

#include "common/robot_kinematics.h"
#include "common/motor_data.h"
#include "common/imu_data.h"

#include "util/logger.h"

SemaphoreHandle_t kinematics_mutex;
TaskHandle_t kinematics_task_handler;

robot_kinematic kinematics = {0};

/*
    What this task needs
    - Motor (L/R) velocities
    - Imu data (mainly gyro)
    - Chunk task adjustment opinions

    All this shit will be used by other tasks (mainly navigation!)

*/
void kinematics_task(void* param){
    robot_kinematic temp = {0};

    uint64_t last_update = millis();

    while(true){
        uint64_t act_time = millis();
        float delta_time = (float)(act_time - last_update)/1000.f;
        last_update = act_time;
        
        motor_data motor = getMotorData();
        imu_data imu = getImuData();
        
        temp.velocity.linear = motor.linear_velocity;
        temp.velocity.angular = imu.gyro.z;                 // TODO: ADD ANGLE COMPENSATION (Also use gyro and accel to compensate angles relative to the ground)
        
        if(abs(temp.velocity.angular) > DEG_TO_RAD * 2.0f){
            temp.position.heading += temp.velocity.angular * delta_time;
            while(temp.position.heading > PI){temp.position.heading -= TWO_PI;}
            while(temp.position.heading < -PI){temp.position.heading += TWO_PI;}
        }
        temp.position.x += cos(temp.position.heading) * temp.velocity.linear * delta_time;
        temp.position.y += sin(temp.position.heading) * temp.velocity.linear * delta_time;

        if(xSemaphoreTake(kinematics_mutex, 5) == pdTRUE){
            kinematics = temp;
            xSemaphoreGive(kinematics_mutex);
        }

        delay(KINEMATICS_DELAY);
    }
}

robot_kinematic getKinematics(){
    robot_kinematic out;

    if(xSemaphoreTake(kinematics_mutex, 5) == pdTRUE){
        out = kinematics;
        xSemaphoreGive(kinematics_mutex);
    }
    
    return out;
}

