#include "imu_task.h"
#include "util/logger.h"
#include "Wire.h"

#include "common/imu_data.h"
#include "common/configs.h"
#include "Waveshare_10Dof-D.h" 

#include "util/file_read.h"



SemaphoreHandle_t imu_mutex;
TaskHandle_t imu_task_handler;

imu_data reading;

struct {
    struct {
        struct {
            float x;
            float y;
            float z;
        } offset;

        struct {    
            float x;
            float y;
            float z;
        } deadzone;
    } gyro;

    struct {
        struct {
            float x;
            float y;
            float z;
        } offset;

        struct {
            float x;
            float y;
            float z;
        } scale;
    } accel;

    struct {
        struct {
            float x;
            float y;
            float z;
        } offset;

        struct {
            float x;
            float y;
            float z;
        } scale;
    } mag;

} imu_calib;

void get_imu_config(){
    JsonDocument config = getConfigData();

    imu_calib.accel.offset.x = config["imu"]["accel"]["calibration"]["offset"]["x"];
    imu_calib.accel.offset.y = config["imu"]["accel"]["calibration"]["offset"]["y"];
    imu_calib.accel.offset.z = config["imu"]["accel"]["calibration"]["offset"]["z"];
    imu_calib.accel.scale.x = config["imu"]["accel"]["calibration"]["scale"]["x"];
    imu_calib.accel.scale.y = config["imu"]["accel"]["calibration"]["scale"]["y"];
    imu_calib.accel.scale.z = config["imu"]["accel"]["calibration"]["scale"]["z"];

    imu_calib.gyro.offset.x = config["imu"]["gyro"]["calibration"]["offset"]["x"];
    imu_calib.gyro.offset.y = config["imu"]["gyro"]["calibration"]["offset"]["y"];
    imu_calib.gyro.offset.z = config["imu"]["gyro"]["calibration"]["offset"]["z"];

    imu_calib.mag.offset.x = config["imu"]["mag"]["calibration"]["offset"]["x"];
    imu_calib.mag.offset.y = config["imu"]["mag"]["calibration"]["offset"]["y"];
    imu_calib.mag.offset.z = config["imu"]["mag"]["calibration"]["offset"]["z"];
    imu_calib.mag.scale.x = config["imu"]["mag"]["calibration"]["scale"]["x"];
    imu_calib.mag.scale.y = config["imu"]["mag"]["calibration"]["scale"]["y"];
    imu_calib.mag.scale.z = config["imu"]["mag"]["calibration"]["scale"]["z"];

}




void imu_task(void* param) {
    IMU_EN_SENSOR_TYPE motionSensor = IMU_EN_SENSOR_TYPE_NULL;
    IMU_EN_SENSOR_TYPE pressureSensor = IMU_EN_SENSOR_TYPE_NULL;
    
    imuInit(&motionSensor, &pressureSensor);

    if (motionSensor != IMU_EN_SENSOR_TYPE_ICM20948) {
        print_error("ICM-20948 WAS NOT FOUND");
        
        while(motionSensor != IMU_EN_SENSOR_TYPE_ICM20948){
            imuInit(&motionSensor, &pressureSensor);
            delay(1000);
        }
        print_alert("ICM-20948 BACK TO OPERATION");
        //vTaskDelay(portMAX_DELAY);                      // Eventually make it try again and again until it manages to be ok
    }
    
    IMU_ST_ANGLES_DATA angulos;
    IMU_ST_SENSOR_DATA gyro;
    IMU_ST_SENSOR_DATA accel;
    IMU_ST_SENSOR_DATA mag;

    get_imu_config();   // get config :D

    while(true){
        imuDataGet(&angulos, &gyro, &accel, &mag);

        imu_data temp_data;

        temp_data.accel.x = (accel.s16X + imu_calib.accel.offset.x) * imu_calib.accel.scale.x;
        temp_data.accel.y = (accel.s16Y + imu_calib.accel.offset.y) * imu_calib.accel.scale.y;
        temp_data.accel.z = (accel.s16Z + imu_calib.accel.offset.z) * imu_calib.accel.scale.z;

        temp_data.gyro.x = DEG_TO_RAD * (gyro.s16X + imu_calib.gyro.offset.x) / 131.f;
        temp_data.gyro.y = DEG_TO_RAD * (gyro.s16Y + imu_calib.gyro.offset.y) / 131.f;
        temp_data.gyro.z = DEG_TO_RAD * (gyro.s16Z + imu_calib.gyro.offset.z) / 131.f;

        if(xSemaphoreTake(imu_mutex, 5) == pdTRUE){
            reading = temp_data;
            xSemaphoreGive(imu_mutex);
        }

        delay(IMU_DELAY);
    }   
    
}

imu_data getImuData(){
    imu_data data;

    if(xSemaphoreTake(imu_mutex, 5) == pdTRUE){
        data.accel.x = reading.accel.x;
        data.accel.y = reading.accel.y;
        data.accel.z = reading.accel.z;
        
        data.gyro.x = reading.gyro.x;
        data.gyro.y = reading.gyro.y;
        data.gyro.z = reading.gyro.z;
        xSemaphoreGive(imu_mutex);
    }

    return data;
}