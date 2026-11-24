#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>

// ------------ Task Config ---------------------
// Core usages
constexpr int NETWORK_CORE = 0;
constexpr int LIDAR_CORE = 1;
constexpr int CHUNK_CORE = 1;
constexpr int IMU_CORE = 1;
constexpr int MOTOR_CORE = 1;
constexpr int NAVIGATION_CORE = 1;
constexpr int KINEMATICS_CORE = 1;
constexpr int HEALTH_CORE = 1;

// Stack sizes
constexpr int NETWORK_STACK_SIZE = 8192;
constexpr int LIDAR_STACK_SIZE = 4096;
constexpr int CHUNK_STACK_SIZE = 8192;
constexpr int IMU_STACK_SIZE = 4096;
constexpr int MOTOR_STACK_SIZE = 4096;
constexpr int NAVIGATION_STACK_SIZE = 8192;
constexpr int KINEMATICS_STACK_SIZE = 4096;
constexpr int HEALTH_STACK_SIZE = 4096;

// Delay times
constexpr int NETWORK_DELAY = 6;  // In ms
constexpr int LIDAR_DELAY = 2;  // In ms
constexpr int CHUNK_DELAY = 5;  // In ms
constexpr int IMU_DELAY = 5;  // In ms
constexpr int MOTOR_DELAY = 5;  // In ms
constexpr int NAVIGATION_DELAY = 10;  // In ms
constexpr int KINEMATICS_DELAY = 5;  // In ms
constexpr int HEALTH_DELAY = 50;    // In ms

// Priorities
constexpr int NETWORK_PRIORITY = 3;
constexpr int LIDAR_PRIORITY = 5;
constexpr int CHUNK_PRIORITY = 4;
constexpr int IMU_PRIORITY = 5;
constexpr int MOTOR_PRIORITY = 4;
constexpr int NAVIGATION_PRIORITY = 3;
constexpr int KINEMATICS_PRIORITY = 6;
constexpr int HEALTH_PRIORITY = 1;

// -------- Queue Sizes --------------
constexpr int LIDAR_QUEUE_SIZE = 10;    // Number of samples a lidar queue can have

// -------- Network -------- 
constexpr int NETWORK_SERVER_PORT = 80;
constexpr const char* MDNS_NAME = "autocar";

// --------  Navigation ---------
constexpr float NAVIGATION_DESTINATION_MARGIN = 15.f; // Margin in cm

// Chunk
constexpr uint16_t CHUNK_BITS = 7;       // How many bits there are in a chunk
constexpr uint16_t CHUNK_MASK = (1 << (CHUNK_BITS)) - 1;
constexpr uint16_t CHUNK_SIZE = (1 << CHUNK_BITS);     // Chunk side size in Units
constexpr float CHUNK_PRECISION = 4.0f; // In cm
constexpr uint16_t CHUNK_LOAD_OFFSET = 30; // In units
constexpr uint16_t CHUNK_LOAD_HISTERISIS = 10; // In units

constexpr uint16_t CHUNK_MAX_ITERATIONS = 15; 
constexpr float CHUNK_CONVERGE_LIMIT = 0.001f;

// --------  Pins -------- 
// Lidar
constexpr int LIDAR_MOTOR_PIN = 1;

// Motor
constexpr int MOTOR_LEFT_A_PIN = 40;
constexpr int MOTOR_LEFT_B_PIN = 41;
constexpr int MOTOR_LEFT_PWM_PIN = 42;
constexpr int MOTOR_LEFT_ENC_A_PIN = 47;
constexpr int MOTOR_LEFT_ENC_B_PIN = 21;

constexpr int MOTOR_RIGHT_A_PIN = 7;
constexpr int MOTOR_RIGHT_B_PIN = 39;
constexpr int MOTOR_RIGHT_PWM_PIN = 6;
constexpr int MOTOR_RIGHT_ENC_A_PIN = 5;
constexpr int MOTOR_RIGHT_ENC_B_PIN = 4;

constexpr float WHEEL_DIAMETER = 6.175f; // in Cm
constexpr float WHEEL_BASE = 17.0f;    // in Cm

// IMU
constexpr int IMU_INT_PIN = 17;
constexpr int IMU_FSYNC_PIN = 18;

// Alarms
constexpr int BUZZER_PIN = 14;
constexpr int RGB_LED_PIN = 48;

// Battery
constexpr int BATTERY_PIN = 2;
constexpr float BATTERY_CALIB_X0 = 39;   // Analog Read at Voltage 1
constexpr float BATTERY_CALIB_Y0 = 1.2f;   // Voltage 1
constexpr float BATTERY_CALIB_X1 = 1205;   // Analog Read at Voltage 2
constexpr float BATTERY_CALIB_Y1 = 11.5f;   // Voltage 2

#endif // CONFIGS_H