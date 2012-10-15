/*


OpenMoco

Time-lapse Core Engine

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
#include "TimerOne.h"
#include <EEPROM.h>

  // openmoco standard libraries
#include "OMComHandler.h"
#include "OMState.h"
#include "OMCamera.h"
#include "OMMotor.h"
#include "OMMoCoBus.h"
#include "OMMoCoNode.h"

  // serial api version
  
#define SERIAL_VERSION  4

#define SERIAL_TYPE  "OMAXISVX"


 // digital I/O line definitions

  // driver enable pin for RS485 bus
#define DE_PIN       5
#define DEBUG_PIN    18
#define CAM_SHT_PIN  13
#define CAM_FOC_PIN  12 
#define AEN_PIN      17
#define STEP_PIN     9
#define DIR_PIN      4
#define MS1_PIN      14
#define MS2_PIN      15
#define MS3_PIN      16
#define PBT_PIN      7


// declare functions that pre-processor can't handle...

void eeprom_write( int pos, byte& val, byte len );
void eeprom_write( int pos, unsigned int& val );
void eeprom_write( int pos, unsigned long& val );
void eeprom_write( int pos, float& val );
void eeprom_write( int pos, byte& val );
void eeprom_read( int pos, byte& val, byte len );
void eeprom_read( int pos, byte& val );
void eeprom_read( int pos, int& val );
void eeprom_read( int pos, unsigned int& val );
void eeprom_read( int pos, unsigned long& val );
void eeprom_read( int pos, float& val );



 // program timer counters

unsigned long run_time    = 0;
unsigned long last_time   = 0;


boolean running = false;

  // do we generate timing for all devices on the network?
  // i.e. -are we the timing master?
boolean timing_master = true;

boolean debug_led_enable = false;

 // device address
byte device_address = 2;

 // necessary camera control variables
 
unsigned int  camera_fired     = 0;

 // motor planned continuous move variables
boolean mtpc       = false;
boolean mtpc_start  = false;

 // maximum run time
unsigned long max_time = 0;

 /*  state transitions
 
  ST_BLOCK - do not allow any action to occur (some event is in process, block the state engine)
  ST_CLEAR - clear to start cycle
  ST_MOVE  - clear to move motor
  ST_RUN   - motor is currently running
  ST_EXP   - clear to expose camera (or not...)
  ST_WAIT  - in camera wait 
  
 */
 
#define ST_BLOCK  0
#define ST_CLEAR  1
#define ST_MOVE   2
#define ST_RUN    3
#define ST_EXP    4
#define ST_WAIT   5

 // initialize core objects
OMCamera     Camera = OMCamera();
OMMotor      Motor  = OMMotor();
OMMoCoNode   Node   = OMMoCoNode(Serial, DE_PIN, device_address, SERIAL_VERSION, SERIAL_TYPE);
OMComHandler ComMgr = OMComHandler();
    // there are 6 possible states in 
    // our engine (0-5)
OMState      Engine = OMState(6);

/* 

 =========================================
 Setup and loop functions
 =========================================
 
*/



void setup() {


  pinMode(DEBUG_PIN, OUTPUT);
  pinMode(PBT_PIN, INPUT);
  
  digitalWrite(PBT_PIN, HIGH);
  
  if( eeprom_saved() == true )
    restore_eeprom_memory();

  // initalize state engine
 setupControlCycle();
 Engine.state(ST_BLOCK);
 
   // default to master timing node
 ComMgr.master(true);
   // set handler for watched common lines
 ComMgr.watchHandler(motor_com_line);
 
   // setup camera defaults
   
 Camera.exposeTime(100);
 Camera.waitTime(0);
 Camera.focusTime(0);
 
 Camera.setHandler(camCallBack);

  // setup serial connection  
 Serial.begin(OM_SER_BPS);

  // setup MoCoBus Node object
 Node.address(device_address);
 Node.setHandler(serCommandHandler);
 Node.setBCastHandler(serBroadcastHandler);
 
  // Listen for address change
 Node.addressCallback(changeNodeAddr);
 
  
  // defaults for motor
 Motor.enable(true);
 Motor.continuous(false);
 Motor.maxStepRate(5000);
 Motor.maxSpeed(800);
 Motor.sleep(true);
 
}





void loop() {


   // check to see if we have any commands waiting      
  Node.check();
     
      // if our program is currently running...
      
   if( running ) {
     
            // update run timer
     unsigned long cur_time = millis();  
     run_time += cur_time - last_time;
     last_time = cur_time;

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


void changeNodeAddr(byte addr) {
   // handle change device address
   // command
  device_address = addr;
  eeprom_write(1, device_address);
}


void pauseProgram() {
     // pause program
  Camera.stop();
  Motor.stop();
  running = false;
}

void stopProgram() {
              
   // stop/clear program
  running      = false;
  run_time     = 0;
  camera_fired = 0;
  mtpc_start   = false;
  
    // clear out motor moved data
    // and stop motor  
  Motor.clear();
  Camera.stop();
}

void startProgram() {
  
     // start program
  last_time      = millis();
  running = true;
  
    // set ready to check for camera
    // we only do thhis for master nodes, not slaves
    // as slaves get their ok to fire state from OMComHandler
  if( ComMgr.master() == true )
    Engine.state(ST_CLEAR); 
                    
}

