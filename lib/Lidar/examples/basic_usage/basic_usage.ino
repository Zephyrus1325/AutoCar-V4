#include "Lidar.h"

/*--------------------------------------------------------------------------+
 *                  Basic lidar usage example                               |      
 *                  Marco Aurélio - 26/04/2026                              |
 *                                                                          | 
 *  This example shows how to setup and show the lidar data                 | 
 *                                                                          |
 *  Connections:                                                            |
 *  - Connect Serial RX to the lidar TX pin                                 |
 *  - Connect motor_pin to a transistor base, controlling the lidar motor   |
 *                                                                          |
 *                                                                          | 
 *-------------------------------------------------------------------------*/

Lidar lidar;

const int motor_pin = 5;

int last_reading_id = 0;

void setup(){
	lidar.begin(&Serial, motor_pin);
	Serial.begin(9600);
}

void loop(){
	lidar.update();			
	
	LidarData lidar_reading = lidar.read();
	
	if(lidar_reading.id() != last_reading_id){
		Serial.print("Initial Angle: ");
		Serial.print(lidar_reading.initialAngle());
		for(int i = 0; i < lidar_reading.length(); i++){
			Serial.print(" | ");
            Serial.print(lidar_reading.distance(i));
		}
        Serial.println();
	}
	
}