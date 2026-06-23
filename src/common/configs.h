#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>

// ------------ Task Config ---------------------
// Core usages
constexpr int NETWORK_CORE = 1;
constexpr int LIDAR_CORE = 1;
constexpr int CHUNK_CORE = 1;
constexpr int IMU_CORE = 1;
constexpr int MOTOR_CORE = 1;
constexpr int NAVIGATION_CORE = 1;
constexpr int KINEMATICS_CORE = 1;

// Stack sizes
constexpr int NETWORK_STACK_SIZE = 8192;
constexpr int LIDAR_STACK_SIZE = 4096;
constexpr int CHUNK_STACK_SIZE = 8192;
constexpr int IMU_STACK_SIZE = 4096;
constexpr int MOTOR_STACK_SIZE = 4096;
constexpr int NAVIGATION_STACK_SIZE = 8192;
constexpr int KINEMATICS_STACK_SIZE = 4096;

// Delay times
constexpr int NETWORK_DELAY = 10;  // In ms
constexpr int LIDAR_DELAY = 2;  // In ms
constexpr int CHUNK_DELAY = 100;  // In ms
constexpr int IMU_DELAY = 5;  // In ms
constexpr int MOTOR_DELAY = 5;  // In ms
constexpr int NAVIGATION_DELAY = 100;  // In ms
constexpr int KINEMATICS_DELAY = 5;  // In ms

// Priorities
constexpr int NETWORK_PRIORITY = 1;
constexpr int LIDAR_PRIORITY = 1;
constexpr int CHUNK_PRIORITY = 1;
constexpr int IMU_PRIORITY = 1;
constexpr int MOTOR_PRIORITY = 1;
constexpr int NAVIGATION_PRIORITY = 1;
constexpr int KINEMATICS_PRIORITY = 1;

// -------- Queue Sizes --------------
constexpr int LIDAR_QUEUE_SIZE = 10;    // Number of samples a lidar queue can have

// -------- Network -------- 
constexpr int NETWORK_SERVER_PORT = 80;
constexpr const char* MDNS_NAME = "autocar";

// --------  Navigation ---------

// Chunk
constexpr uint16_t CHUNK_SIZE = 256;     // Chunk side size in Units
constexpr uint16_t CHUNK_PRECISION = 40; // In mm
constexpr uint16_t CHUNK_LOAD_OFFSET = 30; // In units
constexpr uint16_t CHUNK_LOAD_HISTERISIS = 10; // In units

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

constexpr float WHEEL_DIAMETER = 6.7f; // in Cm
constexpr float WHEEL_BASE = 17.0f;    // in Cm

// IMU
constexpr int IMU_INT_PIN = 17;
constexpr int IMU_FSYNC_PIN = 18;

// Honker
constexpr int BUZZER_PIN = 14;

constexpr int BATTERY_PIN = 2;

#endif // CONFIGS_H