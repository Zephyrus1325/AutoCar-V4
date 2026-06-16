#ifndef MOTOR_LIB_H
#define MOTOR_LIB_H

#include <Arduino.h>

class Motor{
    public:
    Motor(uint16_t a, uint16_t b, uint16_t pwm) : pin_a_(a), pin_b_(b), pin_pwm_(pwm), pin_encoder_a_(-1), pin_encoder_b_(-1){};
    Motor(uint16_t a, uint16_t b, uint16_t pwm, uint16_t enc_a, uint16_t enc_b) : pin_a_(a), pin_b_(b), pin_pwm_(pwm), pin_encoder_a_(enc_a), pin_encoder_b_(enc_b){};

    void begin();
    void update();
    void IRAM_ATTR readEncoder();

    void setThrottle(float throttle);
    void setRPM(float rpm);

    void setPID(float kp, float ki, float kd);

    float getRPM();
    float getSetpoint();
    float getThrottle();
    //private

    int16_t pin_a_;
    int16_t pin_b_;
    int16_t pin_pwm_;
    int16_t pin_encoder_a_;
    int16_t pin_encoder_b_;

    float setpoint_;
    float throttle_;

    float kp_, ki_, kd_;

    volatile uint32_t lastTime_;
    
    uint32_t last_update = 0;
    float integral = 0;
    float last_rpm = 0;


    #define MOTOR_FILTER_SIZE 50
    volatile uint32_t delta_index = 0;
    volatile int32_t deltaTimes_[MOTOR_FILTER_SIZE];
    volatile int8_t directions_[MOTOR_FILTER_SIZE];
};

#endif // MOTOR_LIB_H