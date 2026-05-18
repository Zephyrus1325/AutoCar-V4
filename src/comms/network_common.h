#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <Arduino.h>
#include <WiFi.h>

#include <AsyncUDP.h>
#include <AsyncTCP.h>

// Variables Shared Between the networked stuff
extern AsyncUDP udp_chunk;
extern AsyncUDP udp_lidar;
extern AsyncUDP udp_telemetry;
extern AsyncServer* tcp;
extern AsyncClient* remote;

extern IPAddress remote_pc_ip;

enum udpPacketType{             // param:
    UDP_PACKET_COMMAND,                    // commandType
    UDP_PACKET_MOTOR_INFO,                  // motorData
    UDP_PACKET_LIDAR_INFO,                  // lidarData
    UDP_PACKET_IMU_INFO,                    // IMUData
    UDP_PACKET_BATTERY_INFO,                // BatteryData
    UDP_NETWORK_INFO,                // NetworkData
    UDP_NAVIGATION_INFO,             // NavigationData
};

enum tcpCommandsTypes{          // Param:
    honk,                       // Honk (honk) 
    setIMULowPassFilter,        // uint8_t (0-6)
    setAccelSampleRate,         // uint8_t (0-4)
    setGyroSampleRate,          // uint8_t (0-4)
    setLidarRpm,                // uint16_t (0-450)
    setLidarSampleRate,         // uint8_t (16-32)
    requestIMUConfig,           // None
    requestNetworkConfig,       // None
    requestNavigationConfig,    // None
};

enum udpCommandStream{          // Param
    UDP_STREAM_HANDSHAKE,                  // CALLBACK_MSG
};


#endif // NETWORK_COMMON_H