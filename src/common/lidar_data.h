#ifndef LIDAR_DATA_H
#define LIDAR_DATA_H

#include "Lidar.h"

typedef enum {
    LIDAR_WRONG_SPEED,
    LIDAR_NORMAL
} lidar_info_type;


// Allocate about like 330 bytes, I dunno, lol
typedef struct {
    uint16_t type;      // Making my life easier, again
    uint16_t num_samples;
    float data[];       // Angles in rad | Distances in cm
} __attribute__((packed)) lidar_packet;

typedef struct {
    float throttle;         // in percentage (0.0f to 1.0f)
    uint16_t rpm;           // In rpm ._.
    uint16_t setpoint;      // in RPM
    lidar_info_type type;        // Some metadata to know if theres a packet of data or only simple things
} lidar_info;


// Lidar data is a special kind of data where the tasks will need to process only new data
// so makes sense to make them subscribe to a lidar data publisher 
void subscribe_lidar(bool* flag_ptr); 
void getLidarData(lidar_packet* buffer);
lidar_info getLidarInfo();

#endif // LIDAR_DATA_H