#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

constexpr int UDP_CHUNK_PORT = 8000;
constexpr int UDP_LIDAR_PORT = 8001;
constexpr int UDP_TELEMETRY_PORT = 8002;
constexpr int TCP_COMMAND_PORT = 1225;

constexpr const char* UDP_DISCOVERY_MESSAGE = "HELLO AUTOCAR V4";
constexpr const char* UDP_CONFIRM_MESSAGE = "HELLO REMOTE PC";

constexpr int NETWORK_PING_WAIT = 10000; // How many millis to wait between each ping
constexpr int MAX_RECONNECT_TRIES = 10;

constexpr int TCP_LENGTH_PARSE_BUFFER = 1500;   // 1500 bytes seems enough

#endif // NETWORK_CONFIG_H