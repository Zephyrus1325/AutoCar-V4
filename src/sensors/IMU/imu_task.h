#ifndef IMU_TASK_H
#define IMU_TASK_H

#include <Arduino.h>

typedef struct {
    float x;
    float y;
    float z;
} vec3_t;

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
} vec16_t;

typedef struct {
    vec16_t gyro;
    vec16_t accel;
} imu_reading_t;

typedef struct {
    imu_reading_t reading;

    vec16_t gyro_offset;
    vec3_t gyro_gain;
    
    vec16_t accel_offset;
    vec3_t accel_gain;

    uint8_t low_pass_filter;
    uint8_t accel_range;
    uint8_t gyro_range;
} imu_data_t;

class IMU{
    public:
    IMU();
    update();

    read();


    private:
    imu_reading_t reading[2];

    uint8_t reading_id;

    uint8_t low_pass_filter;
    uint8_t accel_range;
    uint8_t gyro_range;
};

void imu_task(void* param);



#endif // IMU_TASK_H