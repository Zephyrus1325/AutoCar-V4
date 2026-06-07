#include "imu_task.h"

// IMU Configs 
constexpr uint8_t GYRO_SAMPLE_RATE = 0;
constexpr uint8_t GYRO_RANGE = 0;

constexpr uint8_t ACC_SAMPLE_RATE = 0;
constexpr uint8_t ACC_RANGE = 0;

MPU6050 mpu;
TaskHandle_t imu_task_handler;

void imu_task(void* param){
    // IMU Setup
    int16_t ax, ay, az, gx, gy, gz;    // Raw Values

    Wire.begin();
    mpu.initialize();

    while(true){
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);



        char b1[8];
        char b2[8];
        char b3[8];
        char b4[8];
        char b5[8];
        char b6[8];

        sprintf(b1, "%5d", ax);
        sprintf(b2, "%5d", ay);
        sprintf(b3, "%5d", az);
        sprintf(b4, "%5d", gx);
        sprintf(b5, "%5d", gy);
        sprintf(b6, "%5d", gz);


        Serial.print("Accel: ");
        Serial.print(b1);
        Serial.print(", ");
        Serial.print(b2);
        Serial.print(", ");
        Serial.print(b3);
        Serial.print(" | Gyro: ");
        Serial.print(b4);
        Serial.print(", ");
        Serial.print(b5);
        Serial.print(", ");
        Serial.println(b6);

        vTaskDelay(50); // Sample rate of 100 Hz for now
    }
}