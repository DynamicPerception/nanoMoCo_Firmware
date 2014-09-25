/*


Motion Engine

See dynamicperception.com for more information


(c) 2008-2012 C.A. Church / Dynamic Perception LLC

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


#include <MsTimer2.h>
#include <TimerOne.h>
#include <EEPROM.h>

#include <AltSoftSerial.h>

  // openmoco standard libraries
#include <OMComHandler.h>
#include <OMMotorMaster.h>
#include <OMMotorFunctions.h>
#include <OMState.h>
#include <OMCamera.h>
#include <OMMotor.h>
#include <OMMoCoBus.h>
#include <OMMoCoNode.h>
#include <OMEEPROM.h>



const char SERIAL_TYPE[] = "OMAXISVX";

  // serial api version
const byte SERIAL_VERSION = 6;

  // # of flashes of debug led at startup
const byte START_FLASH_CNT = 5;
const byte FLASH_DELAY     = 250;

  // # of milliseconds PBT must be held low to do a factory reset
const unsigned int START_RST_TM = 5000;

const byte CAM_DEFAULT_EXP      = 120;
const byte CAM_DEFAULT_WAIT     = 0;
const byte CAM_DEFAULT_FOCUS    = 0;

const unsigned int MOT_DEFAULT_MAX_STEP  = 1000;
const unsigned int MOT_DEFAULT_MAX_SPD   = 800;

 // digital I/O line definitions

const byte DE_PIN					= 28;
const byte DEBUG_PIN				= 12;
const byte VOLTAGE_PIN				= 42;
const byte CURRENT_PIN				= 41;
const byte BLUETOOTH_ENABLE_PIN		= 0;






/* 
 Need to declare these as early as possible
 
 *******************************
 Mapping of Data Positions in EEPROM memory
 *******************************

 (position count starts at zero)
 
 
 dev_addr        = 0
 name            = 2
 
*/

const int EE_ADDR       = 0; // device_address
const int EE_NAME       = 2; // device name (16 bytes)

// default device address
byte device_address = 3;

//Motor variables
boolean ISR_On = false;

// predefine this function to declare the default argument
void stopProgram(boolean force_clear = true);


 // program timer counters

unsigned long run_time    = 0;
unsigned long last_time   = 0;
boolean running = false;
volatile byte force_stop = false;

//Variables for manual move, if manualMove is true the system expects a command at least once
//every manualMoveTimeMax (mS), if it doesn't receive a command it'll stop the motors
boolean manualMove = false;
const int manualMoveTimeMax = 1000;
unsigned long commandTime = 0;

  // do we generate timing for all devices on the network?
  // i.e. -are we the timing master?
boolean timing_master = true;

boolean debug_led_enable = false;
boolean stepReady = false;
char byteFired = 0;



 // necessary camera control variables
 
unsigned int  camera_fired     = 0;



 // maximum run time
unsigned long max_time = 0;

 // default device name, exactly 15 characters + null terminator
byte device_name[] = "DEFAULT        ";


 // default node to use (Hardware Serial = 1; AltSoftSerial = 2)
byte node =1;

 /*  state transitions
 
  ST_BLOCK - do not allow any action to occur (some event is in process, block the state engine)
  ST_CLEAR - clear to start cycle
  ST_MOVE  - clear to move motor
  ST_RUN   - motor is currently running
  ST_EXP   - clear to expose camera (or not...)
  ST_WAIT  - in camera wait 
  ST_ALTP  - check for alt output post 
  
 */
 
const byte ST_BLOCK = 0;
const byte ST_CLEAR = 1;
const byte ST_MOVE  = 2;
const byte ST_RUN   = 3;
const byte ST_EXP   = 4;
const byte ST_WAIT  = 5;
const byte ST_ALTP  = 6;
unsigned long time  = 0;

 // initialize core objects
OMCamera     Camera = OMCamera();
OMMotorFunctions motor[3] = {
    OMMotorFunctions(OM_MOT1_DSTEP, OM_MOT1_DDIR, OM_MOT1_DSLP, OM_MOT1_DMS1, OM_MOT1_DMS2, OM_MOT1_DMS3, OM_MOT1_STPREG, OM_MOT1_STPFLAG),
 	OMMotorFunctions(OM_MOT2_DSTEP, OM_MOT2_DDIR, OM_MOT2_DSLP, OM_MOT2_DMS1, OM_MOT2_DMS2, OM_MOT2_DMS3, OM_MOT2_STPREG, OM_MOT2_STPFLAG),
 	OMMotorFunctions(OM_MOT3_DSTEP, OM_MOT3_DDIR, OM_MOT3_DSLP, OM_MOT3_DMS1, OM_MOT3_DMS2, OM_MOT3_DMS3, OM_MOT3_STPREG, OM_MOT3_STPFLAG)};

OMMoCoNode   Node   = OMMoCoNode(&Serial, device_address, SERIAL_VERSION, (char*) SERIAL_TYPE);
OMComHandler ComMgr = OMComHandler();
    // there are 6 possible states in 
    // our engine (0-5)
OMState      Engine = OMState(7);

int incomingByte = 0;


/* 

 =========================================
 USB Serial Variables
 =========================================
 
*/

OMMoCoNode   NodeUSB   = OMMoCoNode(&USBSerial, device_address, SERIAL_VERSION, (char*) SERIAL_TYPE);


/* 

 =========================================
 Bluetooth Test Variables
 =========================================
 
*/


AltSoftSerial altSerial;
OMMoCoNode   NodeBlue   = OMMoCoNode(&altSerial, device_address, SERIAL_VERSION, (char*) SERIAL_TYPE);

int timeStart = 0;
int timeEnd = 0;



/* 

 =========================================
 Aux I/O Variables
 =========================================
 
*/

// I/O modes
const byte         ALT_OFF = 0;		//Turns off the I/O
const byte       ALT_START = 1;		//INPUT  - starts program
const byte        ALT_STOP = 2;		//INPUT  - stops program
const byte      ALT_TOGGLE = 3;		//INPUT  - starts the program if it's stop, stops the program if it's running
const byte      ALT_EXTINT = 4;		//INPUT  - external interrupt that triggers the camera to shoot
const byte         ALT_DIR = 5;		//INPUT  - switch the direction of the motors
const byte  ALT_OUT_BEFORE = 6;		//OUTPUT - triggers output before camera shot
const byte   ALT_OUT_AFTER = 7;		//OUTPUT - triggers output after camera shot
const byte ALT_STOP_MOTORS = 8;		//INPUT  - stops the motors, lets the camera run if it's not done
const byte	  ALT_SET_HOME = 9;		//INPUT  - sets the motor's home position
const byte	   ALT_SET_END = 10;	//INPUT  - sets the motor's end position

// These defines are used for the Limit Switch Pin Change Registers

byte            altInputs[] = { ALT_OFF, ALT_OFF };
unsigned int altBeforeDelay = 100;
unsigned int  altAfterDelay = 100;
unsigned int    altBeforeMs = 1000;
unsigned int     altAfterMs = 1000;
boolean        altForceShot = false;
boolean           altExtInt = false;
byte           altDirection = FALLING;
byte             altOutTrig = HIGH;


/* 

 =========================================
 Setup and loop functions
 =========================================
 
*/



void setup() {

   //altSerial.begin(9600);
	
  USBSerial.begin(9600);
  delay(100);
  
  altSerial.begin(9600);
  time = millis();


  pinMode(DEBUG_PIN, OUTPUT);
  pinMode(BLUETOOTH_ENABLE_PIN, OUTPUT);
  digitalWrite(BLUETOOTH_ENABLE_PIN,HIGH);
  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(CURRENT_PIN, INPUT);

    
  // restore/store eeprom memory
 eepromCheck();

  // initalize state engine
 setupControlCycle();
 Engine.state(ST_BLOCK);
 
   // default to master timing node
 ComMgr.master(true);
   // set handler for watched common lines
 ComMgr.watchHandler(motor_com_line);
 
   // setup camera defaults
 Camera.exposeTime(CAM_DEFAULT_EXP);
 Camera.waitTime(CAM_DEFAULT_WAIT);
 Camera.focusTime(CAM_DEFAULT_FOCUS);
 
 Camera.setHandler(camCallBack);

  // setup serial connection  OM_SER_BPS is defined in OMMoCoBus library
 Serial.begin(OM_SER_BPS);

  // setup MoCoBus Node object
 Node.address(device_address);
 Node.setHandler(serNode1Handler);
 Node.setNotUsHandler(serNotUsNode1Handler);
 Node.setBCastHandler(serBroadcastHandler);
 Node.setSoftSerial(false);
 
  // setup MoCoBus Node object for bluetooth

 NodeBlue.address(device_address);
 NodeBlue.setHandler(serNodeBlueHandler);
 NodeBlue.setNotUsHandler(serNotUsNodeBlueHandler);
 NodeBlue.setBCastHandler(serBroadcastHandler);
 NodeBlue.setSoftSerial(true);


  // setup MoCoBus Node object for USB Serial

  NodeUSB.address(device_address);
  NodeUSB.setHandler(serNodeUSBHandler);
  NodeUSB.setNotUsHandler(serNotUsNodeUSBHandler);
  NodeUSB.setBCastHandler(serBroadcastHandler);
  NodeUSB.setSoftSerial(true);

 
 
  // Listen for address change
 Node.addressCallback(changeNodeAddr);
 
 NodeBlue.addressCallback(changeNodeAddr);
 
 NodeUSB.addressCallback(changeNodeAddr);
 
  
  // defaults for motor
 for( int i = 0; i < 3; i++){
	  motor[i].enable(false);
	  motor[i].continuous(false);
	  motor[i].maxStepRate(MOT_DEFAULT_MAX_STEP);
	  motor[i].maxSpeed(MOT_DEFAULT_MAX_SPD);
	  motor[i].sleep(true);
	  motor[i].ms(4);
	 
 }

 
  // enable limit switch handler
// limitSwitch(true);
 
  // startup LED signal
 flasher(DEBUG_PIN, START_FLASH_CNT);
 //startISR();

 
  
}





void loop() {
	
	if(USBSerial.available())
		incomingByte = 1;
  

   // check to see if we have any commands waiting      
  Node.check();
  NodeBlue.check();
  NodeUSB.check();
/*
   if ((millis()-time) > 500)   
   {   
	   if (USBSerial.available()){
		   	 motor[1].maxStepRate(2000);
	   }	 
	 float a = 1.23456789;
	 USBSerial.print("float is: ");
	 USBSerial.print(a,8);
	 USBSerial.print(" in hex it: ");
	 USBSerial.println(a, HEX);
	 time = millis();
	}
	 /*
     
     //altSerial.print(j);
     int voltage=analogRead(VOLTAGE_PIN);  
     int current=analogRead(CURRENT_PIN);
     USBSerial.print("start time is ");
     USBSerial.print(timeStart);
     USBSerial.print(" end time is ");
     USBSerial.print(timeEnd);
	USBSerial.print(" auxInput[0]: ");
	USBSerial.print(altInputs[0]);
	USBSerial.print(" auxInput[1]: ");
	USBSerial.print(altInputs[1]);
	USBSerial.print(" altExtInt: ");
	USBSerial.println(altExtInt);
	//USBSerial.print(" maxsSpeed is: ");
	//USBSerial.println(motor[0].maxSpeed());

   }
   */

	//Check to see if manual move is on and motors are moving
	//must see a command from the master every second or it'll stop
	if ((motor[0].running() || motor[1].running() || motor[2].running()) && manualMove){
		if(millis() - commandTime > manualMoveTimeMax) {
			stopAllMotors();
		}
	}
   
      
	  	   
	for(int i = 0; i<3; i++){
		if(motor[i].running()){
			motor[i].updateSpline();
		}// end if( motor[i].m_isRun
	} // end for loop
	  	   
	  
	  
	  
	// if our program is currently running...      
   if( running ) {
	   


     
            // update run timer
     unsigned long cur_time = millis();  
     run_time += cur_time - last_time;
     last_time = cur_time;

      // Got an external stop somewhere, that wasn't a command?
     if( force_stop == true )
       stopProgram(false);
       
       // hit max runtime? done!
     if( ComMgr.master() && max_time > 0 && run_time > max_time )
       stopProgram();
       
       // if we're the slave and a interrupt has been triggered by the master,
       // set to clear to fire mode (for multi-node sync)
     if( ComMgr.master() == false && ComMgr.slaveClear() == true ) 
       Engine.state(ST_CLEAR);
     
       // check current engine state and handle appropriately
     Engine.checkCycle();
   }

 
}



void pauseProgram() {
     // pause program
  Camera.stop();
  stopAllMotors();
  running = false;
}



void stopProgram(boolean force_clear) {
              
   // stop/clear program
   
   stopAllMotors();
  if( force_clear == true ) {
    run_time     = 0;
    camera_fired = 0;
	for( int i = 0; i < 3; i++){
		motor[i].mtpc_start = false;
	}

  }

  running      = false;
  
    // clear out motor moved data
    // and stop motor 
	clearAll();
	

  Camera.stop();
}



void startProgram() {
  
     // start program
  last_time = millis();
  running   = true;
  
    // debug pin may have been brought high with a force stop
  if( force_stop == true ) {
    digitalWrite(DEBUG_PIN, LOW);
    force_stop = false;
  }
  
    // set ready to check for camera
    // we only do thhis for master nodes, not slaves
    // as slaves get their ok to fire state from OMComHandler
  if( ComMgr.master() == true )
    Engine.state(ST_CLEAR); 
                    
}


void flasher(byte pin, int count) {
    // flash a pin several times (blink)
    
   for(int i = 0; i < count; i++) {
      digitalWrite(pin, HIGH);
      delay(250);
      digitalWrite(pin, LOW);
      delay(250); 
   }
   
}


