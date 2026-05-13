#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <Arduino.h>
#include <WiFi.h>

#include <AsyncUDP.h>
#include <AsyncTCP.h>

// Variables Shared Between the networked stuff
extern AsyncUDP udp;
extern AsyncServer* tcp;
extern AsyncClient* remote;

extern IPAddress remote_pc_ip;

enum udpPacketType {            // Param:
    handshakeMessage,           // DiscoveryMessage / ConfirmationMessage
    lidarData,
    chunkData,
    commandStream,
};

enum tcpPacketType{             // param:
    command,                    // commandType
    motorInfo,                  // motorData
    lidarInfo,                  // lidarData
    IMUInfo,                    // IMUData
    BatteryInfo,                // BatteryData
    NetworkInfo,                // NetworkData
    NavigationInfo,             // NavigationData
};

enum commandsTypes{             // Param:
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


#endif // NETWORK_COMMON_H