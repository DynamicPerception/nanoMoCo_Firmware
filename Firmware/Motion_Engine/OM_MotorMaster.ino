
/*

Motor Control Library

OpenMoco nanoMoCo Core Engine Libraries

See www.openmoco.org for more information

(c) 2008-2011 C.A. Church / Dynamic Perception LLC

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


*/

#include <HardwareSerial.h>
//#include <OM_MotorMaster.h>

void(*f_motSignal)(uint8_t) = 0;





/** Set Callback Handler for motor master

 As actions start and complete, a callback is used to signal what is happening,
 without having to query the method to see if a given action is still occuring.

 By passing a function pointer to this method, that function will be executed
 at each one of these moments, and passed a single, one byte argument that
 matches the signal which is to be sent.  The template for this function is:

 @code
 void function(byte val) { }
 @endcode

 The following codes are defined:

 @code
 OM_MOT_DONE
      - The motor has stopped moving
 OM_MOT_MOVING
      - The motor has begun moving
 @endcode

  An example of an appropriate callback handler:

 @code
 #include "TimerOne.h"
#include "OMMotorMaster.h"

OMMotorMaster Motor = OMMotorMaster();

void setup() {

  Motor.enable(true);
  Motor.continuous(false);
  Motor.setHandler(motorCallback);

  Serial.begin(19200);

}

void loop() {

 if( ! Motor.running() ) {
  delay(1000);
  Serial.println("Moving!");
  Motor.move(true, 1000);
 }

}

void motorCallback( byte code ) {

  if( code == OM_MOT_DONE ) {
    Serial.println("Got Done!");
    Serial.print("Moved: ");
    unsigned long smoved = Motor.stepsMoved();
    Serial.println(smoved, DEC);
  }
  else {
    Serial.println("Got Begin!");
  }

}
  @endcode

   @param p_Func
 A pointer to a function taking a single byte argument

 */


void setMasterHandler( void(*p_Func)(uint8_t) ) {
	f_motSignal = p_Func;
}

void _fireCallback(uint8_t p_Param) {
	if( f_motSignal != 0 ) {
		f_motSignal(p_Param);
	}
}


/** Set Maximum Stepping Rate

 Sets the maximum rate at which steps can be taken.

 Due to the nature of the timing calculations, you are limited to one of the
 following choices: 5000, 4000, 2000, and 1000.  Any other input will be
 ignored and result in no change.

 Please note: the maximum rate here defines how often the step interrupt
 service routine runs.  The higher the rate you use, the greater the amount
 of CPU that will be utilized to run the stepper.  At the maximum rate, over
 50% of the available processing time of an Atmega328 is utilized at 16Mhz,
 while at the lowest rate, approximately 25% of the processing time is utilized.

 For more information on step timing, see the \ref steptiming "Stepping Cycle" section.

 This rate does not dictate the actual speed of specific moves,
 it defines the highest speed which is achievable for any kind of move.

 @param p_Rate
 The rate specified as a number of steps per second (5000, 4000, 2000, or 1000)
 */

bool maxStepRate( unsigned int p_Rate ) {


	if(  p_Rate != 10000 && p_Rate != 5000 && p_Rate != 4000 && p_Rate != 2000 && p_Rate != 1000 )
		return(false);


    motor[0].maxStepRate(p_Rate);
    motor[1].maxStepRate(p_Rate);
    motor[2].maxStepRate(p_Rate);
	return(true);

}

/** Get Maximum Stepping Rate

 Returns the current maximum stepping rate in steps per second. m_curSampleRate is a static variable, therefore
 we can grab it from any of the motors.

 @return
 Maximum rate in steps per second
 */

unsigned int maxStepRate() {
	return( motor[0].maxStepRate() );
}


/** Stop Immediately

  Immediately stops any movement in progress.

  Triggers the callback with the OM_MOT_DONE argument.
  */

void stopAllMotors() {

        // set motors not moving in async mode

      for (int i = 0; i < MOTOR_COUNT; i++) {
		motor[i].stop();		  
		//update current position to EEPROM
		EEPROM.write(EE_POS_0 + (i) * 16, motor[i].currentPos());
      }
      
      ISR_On = false;

      // signal completion
      _fireCallback(OM_MOT_DONE);
                  // let go of interrupt cycle
      Timer1.detachInterrupt();

}


/** Clear Steps Moved

  Clears the count of steps moved.  Additionally, if the motor is currently
  running, any movement will be immediately stopped, and the callback will
  be executed with the OM_MOT_DONE argument.

  This method does NOT reset the home location for the motor.  It does, however,
  reset any plan currently executing or previously executed.

  */

void clearAll() {

		// stop if currently running

	if( motor[0].running() || motor[1].running() || motor[2].running() )
		stopAllMotors();

	for (int i = 0; i < MOTOR_COUNT; i++){

        motor[i].clear();
	}
}


 // execute an async move, when specifying a direction
void startISR() {
	


	 // is async control not already running?
	 if( !ISR_On ) {
		 
		 	USBSerial.print("Start time: ");
		 	USBSerial.println(millis());
           
           _fireCallback(OM_MOT_MOVING);
    	   Timer1.initialize(motor[0].curSamplePeriod());
           Timer1.attachInterrupt(_runISR);
           ISR_On = true;
	 } // end if not running

}


 // our ISR that is run every time Timer1 triggers, controls stepping
 // of the motor

void _runISR() {
	
	//PORTF |= (1 << motor[2].stpflg);
	//delayMicroseconds(1);
	
	
    //steps all motors at once   
	for(int i = 0; i<MOTOR_COUNT; i++){
		if(motor[i].running()){				    
			motor[i].checkRefresh();
			if (motor[i].checkStep()){
				byteFired |= (1 << motor[i].stpflg);
			}
		} // end if( motor[i].m_isRun

	} // end for loop		
		
	
	//byteFired |= (1 << motor[0].stpflg);
    PORTF |= byteFired;
    delayMicroseconds(1);
    PORTF &= ~byteFired;
	

	
	//resets the byteFired flag
	byteFired = 0;
	


    if (!(motor[0].running() || motor[1].running() || motor[2].running())){
        stopAllMotors();
			USBSerial.print("End time: ");
			USBSerial.println(millis());
    }

	//PORTF &= ~(1 << motor[2].stpflg);

}



