#include "comms/network_common.h"

// Variables Shared Between the networked stuff
AsyncUDP udp;
AsyncServer* tcp = NULL;
AsyncClient* remote = NULL;

IPAddress remote_pc_ip;
