#include "comms/network_common.h"

// Variables Shared Between the networked stuff
AsyncUDP udp_chunk;
AsyncUDP udp_lidar;
AsyncUDP udp_telemetry;
AsyncServer* tcp = NULL;
AsyncClient* remote = NULL;

IPAddress remote_pc_ip;
