#ifndef KINEMATICS_H
#define KINEMATICS_H

#include <Arduino.h>

typedef struct {
    struct{
        float x;            // in cm
        float y;            // in cm
        float heading;      // in rad
    } position;
    
    struct{
        float linear;      // in cm/s
        float angular;     // in rad/s
    } velocity;
} robot_kinematic;

robot_kinematic getKinematics();
void setPositionCorrection(float delta_x, float delta_y, float delta_heading);
void setPosition(float x, float y, float heading);
#endif // KINEMATICS_H