#ifndef IMU_DATA_H
#define IMU_DATA_H

typedef struct {
    struct {
        float x;    // in cm/s² ???
        float y;    // in cm/s² ???
        float z;    // in cm/s² ???
    } accel;

    struct {
        float x;    // in rad/s
        float y;    // in rad/s    
        float z;    // in rad/s
    } gyro;

} imu_data;

imu_data getImuData();

#endif // IMU_DATA_H