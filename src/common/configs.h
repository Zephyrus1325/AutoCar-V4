#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>

// Core usages
constexpr int NETWORK_CORE = 1;
constexpr int LIDAR_CORE = 1;
constexpr int CHUNK_CORE = 1;
constexpr int IMU_CORE = 1;
constexpr int MOTOR_CORE = 1;
constexpr int NAVIGATION_CORE = 1;

// Stack sizes
constexpr int NETWORK_STACK_SIZE = 8192;
constexpr int LIDAR_STACK_SIZE = 4096;
constexpr int CHUNK_STACK_SIZE = 8192;
constexpr int IMU_STACK_SIZE = 4096;
constexpr int MOTOR_STACK_SIZE = 4096;
constexpr int NAVIGATION_STACK_SIZE = 8192;

// Network
constexpr int NETWORK_SERVER_PORT = 80;
constexpr const char* MDNS_NAME = "autocar";

// Navigation
// Chunk
constexpr uint16_t CHUNK_SIZE = 256;     // Chunk side size in Units
constexpr uint16_t CHUNK_PRECISION = 40; // In mm
constexpr uint16_t CHUNK_LOAD_OFFSET = 30; // In units
constexpr uint16_t CHUNK_LOAD_HISTERISIS = 10; // In units


// Pins
// Lidar
constexpr int LIDAR_MOTOR_PIN = 1;

// Motor
constexpr int MOTOR_LEFT_A_PIN = 40;
constexpr int MOTOR_LEFT_B_PIN = 41;
constexpr int MOTOR_LEFT_PWM_PIN = 42;
constexpr int MOTOR_LEFT_ENC_A_PIN = 37;
constexpr int MOTOR_LEFT_ENC_B_PIN = 36;

constexpr int MOTOR_RIGHT_A_PIN = 7;
constexpr int MOTOR_RIGHT_B_PIN = 39;
constexpr int MOTOR_RIGHT_PWM_PIN = 6;
constexpr int MOTOR_RIGHT_ENC_A_PIN = 5;
constexpr int MOTOR_RIGHT_ENC_B_PIN = 4;

constexpr float WHEEL_RADIUS = 33.5f; // in Cm

// IMU
constexpr int IMU_INT_PIN = 17;
constexpr int IMU_FSYNC_PIN = 18;

// Honker
constexpr int BUZZER_PIN = 14;

constexpr int BATTERY_PIN = 2;

#endif // CONFIGS_H