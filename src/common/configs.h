#ifndef CONFIGS_H
#define CONFIGS_H


// Core usages
constexpr int NETWORK_CORE = 1;
constexpr int LIDAR_CORE = 1;

// Stack sizes
constexpr int NETWORK_STACK_SIZE = 8192;
constexpr int UDP_TRANSMIT_STACK_SIZE = 8192;
constexpr int TCP_TRANSMIT_STACK_SIZE = 8192;
constexpr int LIDAR_STACK_SIZE = 4096;

// Navigation
// Chunk
constexpr uint16_t CHUNK_SIZE = 128;
constexpr uint16_t CHUNK_PRECISION = 50; // In mm
constexpr uint16_t CHUNK_LOAD_OFFSET = 30; // In units
constexpr uint16_t CHUNK_LOAD_HISTERISIS = 10; // In units


// Pins
// Lidar
constexpr int LIDAR_MOTOR_PIN = 32;

// Motor
constexpr int MOTOR_LEFT_A_PIN = 18;
constexpr int MOTOR_LEFT_B_PIN = 5;
constexpr int MOTOR_LEFT_PWM_PIN = 19;
constexpr int MOTOR_RIGHT_A_PIN = 2;
constexpr int MOTOR_RIGHT_B_PIN = 4;
constexpr int MOTOR_RIGHT_PWM_PIN = 15;

#endif // CONFIGS_H