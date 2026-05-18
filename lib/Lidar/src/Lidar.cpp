#include "Lidar.h"

#include <Arduino.h>

void Lidar::setRPM(uint16_t RPM){
    // Keep able to shut down the motor, but unable to get to low RPMs
    // TODO: Fix bug where you disable the LIDAR but cant get it back up to spin again
    _target = RPM == 0 ? 0 : constrain(RPM, 150, 450);  
    _mode = MODE_RPM;
}

void Lidar::setMotor(uint8_t throttle){
    _target = throttle;
    _mode = MODE_THROTTLE;
}

void Lidar::setDataRate(uint8_t data_rate){
    uint8_t target_samples = constrain(data_rate, 16, 32);
    _mode = MODE_SAMPLE_RATE;
}

uint16_t Lidar::getSetPoint(){return this->_target;}


void Lidar::begin(HardwareSerial* S, int motor_pin){
    _motor_pin = motor_pin;
    _activeId = 0;
    _readId = 0;
    _throttle = 420;
    setRPM(300);

    serial = S;
    serial->begin(115200);
    pinMode(_motor_pin, OUTPUT);
    analogWriteFrequency(25000);    // Sets a high frequency so theres no audible high pitch noise
    analogWriteResolution(10);
    analogWrite(_motor_pin, _throttle);   // Starts to spin the lidar at a initial speed

    for(int i = 0; i < sizeof(_internalReading)/sizeof(_internalReading[0]); i++){
        _internalReading[i]._distance.reserve(90);      //  Reserves those values
        _internalReading[i]._quality.reserve(45);       //  They are pretty high ones
    }
    
}

LidarData Lidar::read(){
    return _internalReading[_readId];
}

void Lidar::update(){
    while(serial->available() > 0){
        // Makes readings until it finds the 0xAA and 0x00 Sync packets
        uint8_t reading = serial->read();   
        if(_lastByte == 0xAA && reading == 0x00){
            _internalReading[_activeId]._length = serial->read();   // Read total packet length
            serial->readBytes(_serial_buffer, _internalReading[_activeId]._length - 1); // Reads entrire packet
            
            // Check if its valid
            if(_checkCRC()){
                _parse();               // Process all data into a LidarData packet
                _readId = _activeId;    // Makes the new packet the latest readable packet
                _activeId++;            // Goes into next packet

                // Overflow prevention stuff
                if(_activeId >= sizeof(_internalReading)/sizeof(_internalReading[0])){
                   _activeId = 0;
                }

                _updateThrottle();      // Uses new RPM data to control the throttle
            }

        }
        _lastByte = reading;            // Stores last reading
    }

}

void Lidar::_updateThrottle(){
    if(_mode == MODE_RPM){
        if(this->read().rpm() > _target + 3){
            _throttle -= 1;
        } else if(this->read().rpm() < _target - 3){
            _throttle += 1;
        }
    }
        
    _throttle = constrain(_throttle, 0, 1024);
    analogWrite(_motor_pin, _throttle);
}

bool Lidar::_checkCRC(){
    uint16_t crc = 0xAA + _internalReading[_activeId]._length;

    // Calculates CRC Sum via received data 
    for(int i = 0; i < _internalReading[_activeId]._length-3; i++){
        crc += _serial_buffer[i];
    }
    
    // Compares with received CRC
    if(((crc >> 8) == _serial_buffer[_internalReading[_activeId]._length-3]) && ((crc & 0xff) == _serial_buffer[_internalReading[_activeId]._length-2])){
      return true;
    }
    return false;
}

enum messageData{
        SYNC0 = 0,              // SYNC0 AND SYNC1 ARE USELESS, APPARENTLY  
        SYNC1 = 1,              // 
        MESSAGE_TYPE = 2,       // USED TO SPECIFY WHAT MESSAGE WE GET
        SYNC2 = 3,              // 
        PAYLOAD_LEN = 4,        // 
        RPM = 5,                // GIVES RPM/3
        START_ANGLE0 = 8,       // GIVES START_ANGLE*100
        START_ANGLE1 = 9,       // 
        PAYLOAD_START = 10      // START OF OUR PAYLOAD
    };

    enum messageType{
        WRONG_SPEED = 0xAE,
        NORMAL = 0xAD
    };

void Lidar::_parse(){
    _internalReading[_activeId]._rpm = _serial_buffer[RPM] * 3; // For some reason theres this 3 times scaling, apparently
    _internalReading[_activeId]._setPoint = _target;
    _internalReading[_activeId]._throttle = _throttle; 
    _internalReading[_activeId]._id = _sample_id++;
    _internalReading[_activeId]._msgType = _serial_buffer[MESSAGE_TYPE];
    if(_serial_buffer[MESSAGE_TYPE] == NORMAL){

        // Clear old values
        _internalReading[_activeId]._quality.clear();
        _internalReading[_activeId]._distance.clear();

        // Get the initial angle and the number of samples
        // Note: angle is 100 times bigger than real angle
        _internalReading[_activeId]._initialAngle = ((_serial_buffer[START_ANGLE0] << 8) + _serial_buffer[START_ANGLE1]);
        _internalReading[_activeId]._length =  (_serial_buffer[PAYLOAD_LEN] - 5)/3;

        // Iterate for each number of samples
        for(int i = 0; i < _internalReading[_activeId]._length; i++){
            // Get sample quality
            _internalReading[_activeId]._quality.push_back( _serial_buffer[PAYLOAD_START + (i*3)]);

            // Get sample distance in units
            _internalReading[_activeId]._distance.push_back((_serial_buffer[PAYLOAD_START + (i*3) + 1] << 8) + _serial_buffer[PAYLOAD_START + (i*3) + 2]);
        }

    } else {
        // If its at wrong speed, or not even giving out data, give a error value so the user can do something about it
        _internalReading[_activeId]._length = -1;
    }
}

// Returns actual lidar RPM
uint16_t LidarData::rpm(){
    return this->_rpm;
};

// Returns actual lidar Setpoint
uint16_t LidarData::setPoint(){
    return this->_setPoint;
};

// Returns actual lidar Throttle
uint16_t LidarData::throttle(){
    return this->_throttle;
};

// Returns unique reading id
uint16_t LidarData::id(){
    return this->_id;
};

// Returns initial reading angle in degrees
float LidarData::initialAngle(){
    return this->_initialAngle * 0.01;
};

// Returns distance in cm
float LidarData::distance(uint16_t index){
    return this->_distance[index] * 0.025f;
};

// Returns quality from the reading
uint8_t LidarData::quality(uint16_t index){
    return this->_quality[index];
};

// Returns the number of sample points in this reading
int16_t LidarData::length(){
    return this->_length;
};

// Returns the number of sample points in this reading
uint8_t LidarData::msgType(){
    return this->_msgType;
};
