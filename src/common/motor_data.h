#ifndef MOTOR_DATA_H
#define MOTOR_DATA_H

typedef struct {
    struct {
        float rpm;              // in rotations per second (are you stupid?)
        float setpoint;         // in cm/s 
        float throttle;         // in percent (-1.0 to 1.0)
        float linear_velocity;  // in cm/s
    } left;

    struct {
        float rpm;              // in rotations per second (are you stupid?)
        float setpoint;         // in cm/s 
        float throttle;         // in percent (-1.0 to 1.0)
        float linear_velocity;  // in cm/s
    } right;
    
    float linear_velocity;      // in cm/s
    float angular_velocity;     // in rad/s
} motor_data;

motor_data getMotorData();
void setMotorSpeed(float left, float right);    // Sets motor linear velocity (in cm/s)

#endif // MOTOR_DATA_H
