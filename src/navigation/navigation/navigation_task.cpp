#include "navigation_task.h"
#include "common/configs.h"
#include "common/navigation.h"

#include "common/robot_kinematics.h"
#include "common/motor_data.h"
#include "util/logger.h"

SemaphoreHandle_t navigation_mutex;
TaskHandle_t navigation_task_handler;
bool isNavigating = false;

float destination_x = 0;
float destination_y = 0;

void setDestination(float dest_x, float dest_y){
    if(xSemaphoreTake(navigation_mutex, 5) == pdTRUE){
        isNavigating = true;
        destination_x = dest_x;
        destination_y = dest_y;
        Serial.printf("Got Destination %4.2f | %4.2f\n\r", dest_x, dest_y);
        xSemaphoreGive(navigation_mutex);
    }
}



void navigation_task(void* param){
    while(true){
        while(isNavigating){
            robot_kinematic robot = getKinematics();
                
            float delta_x = 0;
            float delta_y = 0;

            if(xSemaphoreTake(navigation_mutex, 5) == pdTRUE){
                delta_x = destination_x - robot.position.x;
                delta_y = destination_y - robot.position.y;
                xSemaphoreGive(navigation_mutex);
            } else {continue;}

            float distance = sqrtf(delta_x * delta_x + delta_y * delta_y);
            if(distance < NAVIGATION_DESTINATION_MARGIN){isNavigating = false; setMotorSpeed(0,0); break;}

            float delta_h = atan2(delta_y, delta_x) - robot.position.heading;
            float delta_heading = atan2(sin(delta_h), cos(delta_h));
            //Serial.printf("%4.2f | %4.2f / %4.2f \n\r", delta_x, delta_y, delta_heading);

            //print_log(String(delta_x) + " | " + String(delta_y) + " | " + String(delta_heading));

            float forward = 40.f - abs(delta_heading * 13.f);

            float speed_left = forward - (delta_heading * 8.f);
            float speed_right = forward + (delta_heading * 8.f);
            //Serial.printf("%4.2f | %4.2f\n\r", speed_left, speed_right);
            setMotorSpeed(speed_left, speed_right);

            delay(NAVIGATION_DELAY);
        }
        
        delay(1000);
    }
}

bool getNavigating(){
    bool status = false;
    if(xSemaphoreTake(navigation_mutex, 5) == pdTRUE){
        status = isNavigating;
        xSemaphoreGive(navigation_mutex);
    }
    return status;
}