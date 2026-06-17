#include <Arduino.h>
#include "Motor.h"

void Motor::begin(){
    pinMode(this->pin_a_, OUTPUT);
    pinMode(this->pin_b_, OUTPUT);
    pinMode(this->pin_pwm_, OUTPUT);
    if(this->pin_encoder_a_ >= 0){pinMode(this->pin_encoder_a_, INPUT);}
    if(this->pin_encoder_b_ >= 0){pinMode(this->pin_encoder_b_, INPUT);}

    analogWriteFrequency(25000);    // Sets a high frequency so theres no audible high pitch noise
    analogWriteResolution(10);
    analogWrite(this->pin_pwm_, 0);

    throttle_ = 0;
    for(int i = 0; i < MOTOR_FILTER_SIZE; i++){
        deltaTimes_[i] = 1000000;
    }
    lastTime_ = 200000;
    
    kp_ = 0;
    ki_ = 0;
    kd_ = 0;
}

void Motor::update(){
    float act_rpm = getRPM();
    uint64_t act_time = millis();

    if (act_time == last_update) return;    // deltaTime == 0 protection
    const float max_integral = 1000;

    float deltaTime = float(act_time - last_update)/1000.f;
    float error = setpoint_ - act_rpm;

    float p_out = kp_ * error;

    integral += error * deltaTime;
    integral = constrain(integral, -max_integral, max_integral);
    float i_out = ki_ * integral;

    float d_out = kd_ * ((act_rpm - last_rpm)/deltaTime);
    float out = constrain(p_out + i_out + d_out, -1.f, 1.f);
    
    last_rpm = act_rpm;
    last_update = act_time;
    setThrottle(out);
}

void Motor::setRPM(float rpm){
    this->setpoint_ = rpm;
    if(rpm == 0.f){
        integral = 0;
    }
}

void Motor::setPID(float kp, float ki, float kd){
    this->kp_ = kp;
    this->ki_ = ki;
    this->kd_ = kd;
}

// -1 to 1 
void Motor::setThrottle(float throttle){
    throttle = constrain(throttle, -1.0f, 1.0f);

    if(throttle < 0){
        digitalWrite(this->pin_a_, HIGH);
        digitalWrite(this->pin_b_, LOW);
    } else {
        digitalWrite(this->pin_a_, LOW);
        digitalWrite(this->pin_b_, HIGH);
    }
    analogWrite(this->pin_pwm_, abs(throttle) * 1023);
    throttle_ = throttle;
}

// Ran during interrupts
void IRAM_ATTR Motor::readEncoder(){
    int32_t dir = digitalRead(pin_encoder_b_) ? -1 : 1;
    int32_t delta = (micros() - lastTime_);
    if(delta < 890){return;}   // IGNORE IMPOSSIBLY HIGH SPEEDS
    deltaTimes_[delta_index] = delta;
    directions_[delta_index++] = dir;
    delta_index = delta_index >= MOTOR_FILTER_SIZE ? 0 : delta_index;
    lastTime_ = micros();
}

float Motor::getRPM(){
    int32_t avg_delta = 0;
    int8_t avg_dir = 0;

    int32_t deltas[MOTOR_FILTER_SIZE];
    int8_t dirs[MOTOR_FILTER_SIZE];

    noInterrupts();
    memcpy(deltas, (const void*)deltaTimes_, sizeof(deltaTimes_));
    memcpy(dirs, (const void*)directions_, sizeof(directions_));
    interrupts();

    for(int i = 0; i < MOTOR_FILTER_SIZE; i++){
        avg_delta += deltas[i];
        avg_dir += dirs[i];
    }

    if(avg_dir == 0){return 0.f;}

    avg_dir = avg_dir/abs(avg_dir); // Makes into -1 or 1
    
    avg_delta = avg_delta / MOTOR_FILTER_SIZE;

    if(avg_delta >= 890 && micros() - lastTime_ < 100000){
        return (130000.f/float(avg_delta)) * avg_dir;
    } else {
        return 0.f;
    }
}

float Motor::getSetpoint(){
    return this->setpoint_;
}

float Motor::getThrottle(){
    return this->throttle_;
}