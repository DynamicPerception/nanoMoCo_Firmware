/*
 * OMAxis.h
 *
 *  Created on: 08.05.2012
 *      Author: perepelitsa
 */

#ifndef OMAXIS_H_
#define OMAXIS_H_

#include "OMMoCoMaster.h"

/** OMAxis provides the primary interface for controlling nanoMoCo devices on an AVR platform.
 
 Only one OMAxis instance is required to speak to any number of devices.  For most nanoMoCo-specific
 commands, you need only to specify the target() before issuing a command. Each command issued returns
 true or false indicating whether the remote device successfully received the command.
 
 The following provides a basic example of controlling a node via this class in an Arduino sketch:
 
 @code
 #include "OMMoCoBus.h"
 #include "OMMoCoMaster.h"
 #include "OMAxis.h"
 
  // RS-485 driver enable pin must be pin 5
 #define DE_PIN 5
 
 byte nodeAddr = 3;
 
 OMAxis axis = OMAxis(Serial, DE_PIN);
 
 void setup() {
    Serial.begin(OM_SER_BPS);
 }
 
 void loop() {
 
    static bool isRunning = false;
 
    if( ! isRunning ) {
        axis.target(nodeAddr);
        axis.enableCamera();
        axis.interval(2);
        axis.enableMotor();
        axis.steps(50);
        axis.maxShots(100);
        axis.start();
    }
 
 }
 @endcode
 
 @author Stanislav Pereplitsa
 
 Documentation by C.A. Church
 
 Copyright (c) 2012 Dynamic Perception LLC
 
 This software is shared under the GNU GPLv3 License
 */

class OMAxis: public OMMoCoMaster {



public:
	OMAxis(HardwareSerial& c_serObj, uint8_t c_dePin);
	virtual ~OMAxis();

public:
	//supported commands
	bool interval(uint16_t interval);
	bool exposure(uint32_t exposure);
	bool focus(uint16_t focus);
	bool maxShots(uint16_t count);
	bool exposureDelay(uint16_t delay);
	bool tie(bool isOn);
	bool repeat(uint8_t count);
	bool steps(uint16_t steps);
	bool dir(bool dir);
	bool maxSteps(uint32_t steps);
	bool enableMotor(void);
	bool disableMotor(void);
	bool setHome(void);
	bool backlash(uint8_t steps);
	bool easing(uint8_t mode);
	bool continuous(bool isOn);

    void target(uint8_t p_addr);
    uint8_t target();
    
	//

	bool start();
	bool pause();
	bool enableCamera();
	bool disableCamera();
	bool move(bool dir, uint16_t steps);
    bool move(bool dir, uint32_t steps,
			       uint32_t totalTravelTime, uint32_t accelTime,
			       uint32_t decelTime);
	bool home();
	bool expose(uint32_t exposure);
	bool sleep(bool isSleep);
	bool maxStepRate(uint16_t stepsPerSecond);
	bool maxSpeed(uint16_t stepsPerSecond);
	bool debug(bool isOn);
	bool stopMotor();
	bool ms(uint8_t level);
	bool master(bool isMaster);

	bool plan(bool dir, uint32_t steps,
			                   uint32_t totalTravelIntervals, uint32_t accelIntervals,
			                   uint32_t decelIntervals);
	bool clearPlan();
    
	bool status(uint8_t statusType, unsigned char* blob);

private:
    // internal data
    uint8_t m_slaveAddr;

};

#endif /* OMAXIS_H_ */
