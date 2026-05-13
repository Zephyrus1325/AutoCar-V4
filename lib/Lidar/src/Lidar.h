#ifndef LIDAR_H
#define LIDAR_H

#include <Arduino.h>

#include <vector>

class LidarData {
    public:
    LidarData(){};

    uint8_t msgType();
    uint16_t throttle();
    uint16_t rpm();
    uint16_t setPoint();
    uint16_t id();
    float initialAngle();
    float distance(uint16_t index);
    uint8_t quality(uint16_t index);
    int16_t length();

    uint8_t _msgType;
    uint16_t _throttle;
    uint16_t _rpm;
    uint16_t _setPoint;
    uint16_t _id;
    uint16_t _initialAngle;
    std::vector<uint16_t> _distance;
    std::vector<uint8_t> _quality;
    int16_t _length;
    
};

class Lidar {
    public:
    Lidar() : serial(nullptr){};
    
    void begin(HardwareSerial* S, int motor_pin);

    LidarData read();
    void update();
    
    void setRPM(uint16_t RPM);
    void setMotor(uint8_t throttle);
    void setDataRate(uint8_t data_rate);
    uint16_t getSetPoint();

    private:
    enum LidarMode {
        MODE_RPM,
        MODE_THROTTLE,
        MODE_SAMPLE_RATE
    };
    bool _checkCRC();
    void _parse();
    void _updateThrottle();
    int _motor_pin;
    HardwareSerial* serial;
    uint8_t _lastByte;
    uint8_t _serial_buffer[140];
    LidarData _internalReading[2];
    uint8_t _activeId;
    uint8_t _readId;
    int16_t _throttle;
    uint16_t _target;
    uint8_t _mode;
    uint32_t _sample_id;
};



#endif // LIDAR_H