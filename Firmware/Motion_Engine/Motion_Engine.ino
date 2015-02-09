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
const int SERIAL_VERSION = 25;

  // # of flashes of debug led at startup
const byte START_FLASH_CNT = 5;
const byte FLASH_DELAY     = 250;

  // # of milliseconds PBT must be held low to do a factory reset
const unsigned int START_RST_TM = 5000;

const byte CAM_DEFAULT_EXP      = 120;
const byte CAM_DEFAULT_WAIT     = 0;
const byte CAM_DEFAULT_FOCUS    = 0;

const unsigned int MOT_DEFAULT_MAX_STEP  = 5000;
const unsigned int MOT_DEFAULT_MAX_SPD   = 5000;
const float MOT_DEFAULT_CONT_ACCEL	     = 15000.0;

const unsigned int MOT_DEFAULT_BACKLASH = 0;

 // digital I/O line definitions

const byte DE_PIN					= 28;
const byte DEBUG_PIN				= 12;
const byte VOLTAGE_PIN				= 42;
const byte CURRENT_PIN				= 41;
const byte BLUETOOTH_ENABLE_PIN		= 0;

// motor count constant
const byte MOTOR_COUNT				= 3;

// plan move types
#define SMS 0
#define CONT_TL 1
#define CONT_VID 2

// General computational constants
#define MILLIS_PER_SECOND	1000.0

// Valid microstep settings
#define FULL				1
#define HALF				2
#define QUARTER				4
#define EIGHTH				8
#define SIXTHEENTH			16


bool respond_flag = false;
unsigned int interferences = 0;
unsigned int old_interferences = 0;


/* 
 Need to declare these as early as possible
 
 *******************************
 Mapping of Data Positions in EEPROM memory
 *******************************

 (position count starts at zero)
 
 
 dev_addr        = 0
 name            = 2
 
*/

const int EE_ADDR       = 0;				// device_address (2 bytes)
const int EE_NAME       = 2;				// device name (16 bytes)

const int EE_POS_0   = EE_NAME    + 10;		// Motor 0 current position (long int)
const int EE_END_0   = EE_POS_0   + 4;		// Motor 0 end limit position (long int)
const int EE_START_0 = EE_END_0   + 4;		// Motor 0 program start position (long int)
const int EE_STOP_0  = EE_START_0 + 4;		// Motor 0 program stop position (long int)
const int EE_MS_0    = EE_STOP_0  + 4;		// Motor 0 microstep value (byte)

const int EE_POS_1   = EE_MS_0    + 1;		// Motor 1 current position (long int)
const int EE_END_1   = EE_POS_1   + 4;		// Motor 1 end limit position (long int)
const int EE_START_1 = EE_END_1   + 4;		// Motor 1 program start position (long int)
const int EE_STOP_1  = EE_START_1 + 4;		// Motor 1 program stop position (long int)
const int EE_MS_1    = EE_STOP_1  + 4;		// Motor 1 microstep value (byte)

const int EE_POS_2   = EE_MS_1    + 1;		// Motor 2 current position (long int)
const int EE_END_2   = EE_POS_2   + 4;		// Motor 2 end limit position (long int)
const int EE_START_2 = EE_END_2   + 4;		// Motor 2 program start position (long int)
const int EE_STOP_2  = EE_START_2 + 4;		// Motor 2 program stop position (long int)
const int EE_MS_2    = EE_STOP_2  + 4;		// Motor 2 microstep value (byte)

const int EE_MOTOR_MEMORY_SPACE = 17;		//Number of bytes required for storage for each motor's variables


//temp values for EEPROM
long tempPos = 0;
byte tempMS = 0;  

// default device address
int device_address = 3;

//Motor variables
uint8_t ISR_On = false;

// predefine this function to declare the default argument
void stopProgram(uint8_t force_clear = true);


 // program timer counters

unsigned long run_time    = 0;	// Amount of time since the program has started (ms)
unsigned long last_run_time = 0; // Stores the run time, even after the program has ended
unsigned long last_time   = 0;
uint8_t running = false;
volatile byte force_stop = false;

//Variables for joystick move, if watchdog is true the system expects a command at least once
//every watchdogTimeMax (mS), if it doesn't receive a command it'll stop the motors
uint8_t watchdog = false;
const int watchdogTimeMax = 1000;
unsigned long commandTime = 0;
byte joystick_mode = false;
 

  // do we generate timing for all devices on the network?
  // i.e. -are we the timing master?
uint8_t timing_master = true;

uint8_t debug_led_enable = false;
uint8_t stepReady = false;
char byteFired = 0;



 // necessary camera control variables
unsigned int  camera_fired     = 0;
uint8_t		  camera_test_mode = false;
uint8_t					fps    = true;

// ping pong mode variable
uint8_t ping_pong_mode = false;

 // maximum run time
unsigned long max_time = 0;

// time delay for program starting
unsigned long start_delay = 0;

// pause flag for later call of pauseProgram() 
bool pause_flag = false;

// key frame variables
int key_frames = 0;
int current_frame = 0;
bool key_move = false;

// program completion flag
bool program_complete = false;

 // default device name, exactly 9 characters + null terminator
byte device_name[] = "DEFAULT   ";


 // default node to use (Hardware Serial = 1; AltSoftSerial = 2)
byte node =1;

 /*  state transitions
 
  ST_BLOCK - do not allow any action to occur (some event is in process, block the state engine)
  ST_CLEAR - clear to start cycle
  ST_MOVE  - clear to move motor
  ST_RUN   - motor is currently running
  ST_EXP   - clear to expose camera (or not...)
  ST_WAIT  - in camera delay 
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
OMMotorFunctions motor[MOTOR_COUNT] = {
    OMMotorFunctions(OM_MOT1_DSTEP, OM_MOT1_DDIR, OM_MOT1_DSLP, OM_MOT1_DMS1, OM_MOT1_DMS2, OM_MOT1_DMS3, OM_MOT1_STPREG, OM_MOT1_STPFLAG),
 	OMMotorFunctions(OM_MOT2_DSTEP, OM_MOT2_DDIR, OM_MOT2_DSLP, OM_MOT2_DMS1, OM_MOT2_DMS2, OM_MOT2_DMS3, OM_MOT2_STPREG, OM_MOT2_STPFLAG),
 	OMMotorFunctions(OM_MOT3_DSTEP, OM_MOT3_DDIR, OM_MOT3_DSLP, OM_MOT3_DMS1, OM_MOT3_DMS2, OM_MOT3_DMS3, OM_MOT3_STPREG, OM_MOT3_STPFLAG)};

OMMoCoNode   Node   = OMMoCoNode(&Serial, device_address, SERIAL_VERSION, (char*) SERIAL_TYPE);
OMComHandler ComMgr = OMComHandler();
    // there are 7 possible states in 
    // our engine (0-6)
OMState      Engine = OMState(7);

int incomingByte = 0;

/*

=========================================
Debugging Variable and Associated Flags
=========================================

*/

byte usb_debug			= B00000000;
const byte DB_COM_OUT	= B00000001;
const byte DB_STEPS		= B00000010;
const byte DB_MOTOR		= B00000100;
const byte DB_GEN_SER	= B00010000;
const byte DB_FUNCT		= B00100000;
const byte DB_CONFIRM	= B01000000;

/* 

 =========================================
 USB Serial Variables
 =========================================
 
*/

OMMoCoNode   NodeUSB   = OMMoCoNode(&USBSerial, device_address, SERIAL_VERSION, (char*) SERIAL_TYPE);


/* 

 =========================================
 Bluetooth Variables
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
uint8_t        altForceShot = false;
uint8_t           altExtInt = false;
byte           altDirection = FALLING;
byte             altOutTrig = HIGH;

//Temp variable for storing information for node response
int temp = 0;
bool external_intervalometer = false;


/* 

 =========================================
 Setup and loop functions
 =========================================
 
*/



void setup() {

   //altSerial.begin(9600);
	
  USBSerial.begin(19200);
  delay(100);
  
  altSerial.begin(9600);
  time = millis();

  if (usb_debug & DB_FUNCT)
	USBSerial.println("Done setting things up!");


  pinMode(DEBUG_PIN, OUTPUT);
  pinMode(BLUETOOTH_ENABLE_PIN, OUTPUT);
  digitalWrite(BLUETOOTH_ENABLE_PIN,HIGH);
  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(CURRENT_PIN, INPUT);

    


  // initalize state engine
 setupControlCycle();
 Engine.state(ST_BLOCK);
 
   // default to master timing node
 ComMgr.master(true);
   // set handler for watched common lines
 ComMgr.watchHandler(motor_com_line);
 
   // setup camera defaults
 Camera.triggerTime(CAM_DEFAULT_EXP);
 Camera.delayTime(CAM_DEFAULT_WAIT);
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
 for( int i = 0; i < MOTOR_COUNT; i++){
	  motor[i].enable(false);
	  motor[i].maxStepRate(MOT_DEFAULT_MAX_STEP);
	  motor[i].contSpeed(MOT_DEFAULT_MAX_SPD);
	  motor[i].contAccel(MOT_DEFAULT_CONT_ACCEL);
	  motor[i].sleep(false);
	  motor[i].backlash(MOT_DEFAULT_BACKLASH);
	  // Set the slide motor to 4th stepping and pan/tilt motors to 16th
	  if (i == 0)
		  motor[i].ms(4);
	  else
		  motor[i].ms(16);
	  motor[i].programBackCheck(false);	 
 }

  // restore/store eeprom memory
  eepromCheck();
 
  // enable limit switch handler
// limitSwitch(true);
 
  // startup LED signal
 flasher(DEBUG_PIN, START_FLASH_CNT);
 //startISR();


 attachInterrupt(1, eStop, FALLING); 
}




void loop() {
	
	if(USBSerial.available())
		incomingByte = 1;
  

   // check to see if we have any commands waiting      
  Node.check();
  NodeBlue.check();
  NodeUSB.check();

  if ((millis() - time) > 500 && (usb_debug & DB_STEPS))
  {

	  USBSerial.print(motor[0].currentPos());
	  USBSerial.print(" continious Speed: ");
	  USBSerial.print(motor[0].contSpeed());
	  USBSerial.print(" backlash: ");
	  USBSerial.print(motor[0].backlash());
	  USBSerial.print(" startPos: ");
	  USBSerial.print(motor[0].startPos());
	  USBSerial.print(" stopPos: ");
	  USBSerial.print(motor[0].stopPos());
	  USBSerial.print(" endPos: ");
	  USBSerial.print(motor[0].endPos());
	  USBSerial.print(" running: ");
	  USBSerial.print(motor[0].running());
	  USBSerial.print(" enable: ");
	  USBSerial.print(motor[0].enable());
	  USBSerial.print(" Type: ");
	  USBSerial.println(motor[0].planType());
	  USBSerial.print(" shots: ");
	  USBSerial.print(camera_fired);
	  USBSerial.print(" leadIn: ");
	  USBSerial.println(motor[0].planLeadIn());

	  USBSerial.print("Current Steps ");
	  USBSerial.print(motor[1].currentPos());
	  USBSerial.print(" continious Speed: ");
	  USBSerial.print(motor[1].contSpeed());
	  USBSerial.print(" backlash: ");
	  USBSerial.print(motor[1].backlash());
	  USBSerial.print(" startPos: ");
	  USBSerial.print(motor[1].startPos());
	  USBSerial.print(" stopPos: ");
	  USBSerial.print(motor[1].stopPos());
	  USBSerial.print(" endPos: ");
	  USBSerial.print(motor[1].endPos());
	  USBSerial.print(" Type: ");
	  USBSerial.println(motor[1].planType());
	  USBSerial.print(" leadIn: ");
	  USBSerial.println(motor[1].planLeadIn());

	  USBSerial.print("Current Steps ");
	  USBSerial.print(motor[2].currentPos());
	  USBSerial.print(" continious Speed: ");
	  USBSerial.print(motor[2].contSpeed());
	  USBSerial.print(" backlash: ");
	  USBSerial.print(motor[2].backlash());
	  USBSerial.print(" startPos: ");
	  USBSerial.print(motor[2].startPos());
	  USBSerial.print(" stopPos: ");
	  USBSerial.print(motor[2].stopPos());
	  USBSerial.print(" endPos: ");
	  USBSerial.println(motor[2].endPos());
	  USBSerial.println("");
  }
		
		time = millis();
	

	//Check to see if joystick watchdog is on and motors are moving
	//must see a command from the master every second or it'll stop
	if ((motor[0].running() || motor[1].running() || motor[2].running()) && watchdog){
		if(millis() - commandTime > watchdogTimeMax) {
			stopAllMotors();
		}
	}
   
   
   
	for(int i = 0; i<MOTOR_COUNT; i++){
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

		if (run_time != 0)
			last_run_time = run_time;

		// Got an external stop somewhere, that wasn't a command?
		if( force_stop == true )
			stopProgram();
       
		// hit max runtime? done!
		if( ComMgr.master() && max_time > 0 && run_time > max_time )
			stopProgram();
       
		// if we're the slave and a interrupt has been triggered by the master,
		// set to clear to fire mode (for multi-node sync)
		if( ComMgr.master() == false && ComMgr.slaveClear() == true ) 
			Engine.state(ST_CLEAR);
     
		// if the start delay is done then check current engine state and handle appropriately
		if(run_time >= start_delay)
			Engine.checkCycle();
   }

}

void pauseProgram() {
     // pause program
  Camera.stop();
  stopAllMotors();
  running = false;
 // USBSerial.println("Pausing program!!!");
}



void stopProgram(uint8_t force_clear) {
              
   // stop/clear program
   
   stopAllMotors();
  if( force_clear == true ) {
    run_time     = 0;
    camera_fired = 0;
	for( int i = 0; i < MOTOR_COUNT; i++){
		//resets the program move
		motor[i].resetProgramMove();
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
	for( int i = 0; i < MOTOR_COUNT; i++){
		if(motor[i].enable())
			motor[i].programDone(false);
	}
  
    // debug pin may have been brought high with a force stop
  if( force_stop == true ) {
    digitalWrite(DEBUG_PIN, LOW);
    force_stop = false;
  }
  
    // set ready to check for camera
    // we only do this for master nodes, not slaves
    // as slaves get their ok to fire state from OMComHandler
  if (ComMgr.master() == true) {
	  Engine.state(ST_CLEAR);
  }
                    
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

byte powerCycled() {
	
	// This function will respond true the first time it is
	// called after a power cycle and false thereafter
	static byte cycled = true;
	byte response = cycled;
	cycled = false;

	return(response);
}

void eStop() {

	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();

	static unsigned long _time = millis();
	static byte enable_count = 0;
	const byte THRESHOLD = 3;
	
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200) {

		if (running && !camera_test_mode)
			stopProgram();	// This previously paused the running program, but that caused weird state issues with the mobile app
		
		else if (running && camera_test_mode)
			stopProgram();
		
		else if (!motor[0].running() && !motor[1].running() && !motor[2].running()) {
			// If the button was pressed recently enough, increase the enable count, otherwise reset it to 0.
			if (millis() - _time < 1500)
				enable_count++;
			else
				enable_count = 1;
			//USBSerial.print("Switch count ");
			//USBSerial.println(_enable_count);

			// If the user has pressed the e-stop enough times within the alloted time span, enabled the external intervalometer
			if (enable_count >= THRESHOLD && !external_intervalometer) {
				limitSwitchAttach(0);
				altConnect(1, ALT_EXTINT);
				altSetup();
				external_intervalometer = true;
				enable_count = 0;
				
				// Turn the debug light on to confirm the setting
				debugOn();
			}
			else if (enable_count >= THRESHOLD && external_intervalometer) {
				altConnect(1, ALT_OFF);
				altSetup();
				external_intervalometer = false;
				enable_count = 0;

				// Turn the debug light off to confirm the setting
				debugOff();
			}
			time = millis();
		}

		else
			stopAllMotors();
	}
	last_interrupt_time = interrupt_time;	
}

uint8_t programPercent() {
	
	unsigned long time = millis();
	static uint8_t percent = 0;

	unsigned long longest_move = 0;
	
	// Check the total length of each motor's move and save the longest one
	for (byte i = 0; i < MOTOR_COUNT; i++) {
		
		// If the motor isn't enabled, don't check its move length
		if (!motor[i].enable())
			continue;
		
		unsigned long current_move;

		current_move = motor[i].planLeadIn() + motor[i].planTravelLength() + motor[i].planLeadIn();

		// Update the longest move if necessary
		if (current_move > longest_move)
			longest_move = current_move;

	}

	uint8_t percent_new;

	// If in SMS mode and the camera max shots is less than the longest motor move, use that value instead
	if (motor[0].planType() == SMS && Camera.maxShots < longest_move)
		longest_move = Camera.maxShots;

	// Determine the program percent completion by dividing the current shots by the max shots.
	// Multiply by 100 to give whole number percent.

	// Determine the percent completion for SMS based on shots
	if (motor[0].planType() == SMS)
		percent_new = round((float)camera_fired / (float)longest_move * 100.0);

	// Otherwise determin the percent completion based on run-time
	else
		percent_new = round((float)run_time / (float)longest_move * 100.0);

	// If the newly calculated percent complete is 0 and the last percent complete was non-zero, then the program has finished and the program should report 100% completion
	if (percent_new == 0 && percent != 0)
		percent = 100;
	else
		percent = percent_new;
	return(percent);
}


uint8_t checkMotorAttach() {

	// If any of the motors is moving or a program is currently running return the error value
	for (byte i = 0; i < MOTOR_COUNT; i++) {
		if (motor[i].running() || running)
			return(B1000); // B1000 = 8
	}

	bool current_sleep[MOTOR_COUNT];
	uint8_t attached = B000;

	
	for (byte i = 0; i < MOTOR_COUNT; i++) {
		// Save the current sleep state of all the motors so they can be restored when done with the attach check
		current_sleep[i] = motor[i].sleep();
		// Put them into sleep mode in case it isn't already
		motor[i].sleep(true);
	}

	for (byte i = 0; i < MOTOR_COUNT; i++) {
		motor[i].sleep(false);
		delay(100);
		// Read the analog value from current sensing pin
		int current = analogRead(CURRENT_PIN);		
		// Convert the value to current in millamps
		float amps = (float)current / 1023 * 5;
		// This is the threshold in amps above which a motor will register as being detected;
		const float THRESHOLD = 0.15;
		// If the draw is greater than <THRESHOLD> amps, then a motor is connected to the enabled channel
		if (amps > THRESHOLD)
			attached |= (1 << i);
		// Put the motor back to sleep so it doesn't interfere with reading of the next motor
		motor[i].sleep(true);
		//USBSerial.print("Motor ");
		//USBSerial.print(i);
		//USBSerial.print(" current draw: ");
		//USBSerial.println(amps);
	}

	// Restore the saved sleep states
	for (byte i = 0; i < MOTOR_COUNT; i++) {
		motor[i].sleep(current_sleep[i]);
	}

	// The bits of the attached byte indicate each motor's attached status
	return(attached);
}


/**

	Returns the total run time of the currently set program in milliseconds
	
*/
unsigned long totalProgramTime() {

	unsigned long longest_time = 0;
	unsigned long motor_time = 0;

	for (byte i = 0; i < MOTOR_COUNT; i++) {
		// If the motor is enabled, check its program time
		if (motor[i].enable()) {
			// SMS: Total the exposures for the program and multiply by the interval
			if (motor[i].planType() == SMS) {
				motor_time = Camera.interval * (motor[i].planLeadIn() + motor[i].planTravelLength() + motor[i].planLeadOut());
				if (usb_debug & DB_FUNCT){
					USBSerial.print("Interval: ");
					USBSerial.print(Camera.interval);
					USBSerial.print("  Lead in: ");
					USBSerial.print(motor[i].planLeadIn());
					USBSerial.print("  Accel: ");
					USBSerial.print(motor[i].planAccelLength());
					USBSerial.print("  Travel: ");
					USBSerial.print(motor[i].planTravelLength());
					USBSerial.print("  Decel: ");
					USBSerial.print(motor[i].planDecelLength());
					USBSerial.print("  Lead out: ");
					USBSerial.print(motor[i].planLeadOut());
					USBSerial.print("  Motor time: ");
					USBSerial.println(motor_time);
				}
			}
			// CONT_TL AND CONT_VID: all segments are in milliseconds, no need to multiply anything
			else
				motor_time = motor[i].planLeadIn() + motor[i].planTravelLength() + motor[i].planLeadOut();
			// Overwrite longest_time if the last checked motor is longer
			if (motor_time > longest_time)
				longest_time = motor_time;
		}
	}

	return(longest_time);
}

uint8_t programComplete() {

	// This function will respond true the first time it is
	// called after a power cycle and false thereafter

	uint8_t status = false;

	if (program_complete == true)
		status = true;

	program_complete = false;

	return(status);

	
}