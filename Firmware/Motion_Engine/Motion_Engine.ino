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
#include "TimerOne.h"
#include <EEPROM.h>

#include <AltSoftSerial.h>

  // openmoco standard libraries
#include <OMComHandler_AT90USB.h>
#include <OMState.h>
#include <OMCamera.h>
#include <OMMotor.h>
#include <OMMoCoBus.h>
#include <OMMoCoNode.h>
#include <OMEEPROM.h>



const char SERIAL_TYPE[] = "OMAXISVX";

  // serial api version
const byte SERIAL_VERSION = 5;

  // # of flashes of debug led at startup
const byte START_FLASH_CNT = 5;
const byte FLASH_DELAY     = 250;

  // # of milliseconds PBT must be held low to do a factory reset
const unsigned int START_RST_TM = 5000;

const byte CAM_DEFAULT_EXP      = 120;
const byte CAM_DEFAULT_WAIT     = 0;
const byte CAM_DEFAULT_FOCUS    = 0;

const unsigned int MOT_DEFAULT_MAX_STEP  = 5000;
const unsigned int MOT_DEFAULT_MAX_SPD   = 800;

 // digital I/O line definitions

const byte DE_PIN       = 28;//5;
const byte DEBUG_PIN    = 12;//18;
const byte CAM1_SHT_PIN  = 9;
const byte CAM1_FOC_PIN  = 11;
const byte CAM2_SHT_PIN  = 8;
const byte CAM2_FOC_PIN  = 10;
const byte AEN_PIN      = 32;
const byte STEP_PIN     = 45;
const byte DIR_PIN      = 16;
const byte MS1_PIN      = 17;
const byte MS2_PIN      = 18;
const byte MS3_PIN      = 19;
const byte PBT_PIN      = 7;






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



  // predefine this function to declare the default argument
void stopProgram(boolean force_clear = true);


 // program timer counters

unsigned long run_time    = 0;
unsigned long last_time   = 0;


boolean running = false;
volatile byte force_stop = false;

  // do we generate timing for all devices on the network?
  // i.e. -are we the timing master?
boolean timing_master = true;

boolean debug_led_enable = false;

 // default device address
byte device_address = 3;

 // necessary camera control variables
 
unsigned int  camera_fired     = 0;

 // motor planned move variables that need to be defined here
boolean mtpc       = false;
boolean mtpc_start = false;
boolean mt_plan    = false;

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
  
 */
 
const byte ST_BLOCK = 0;
const byte ST_CLEAR = 1;
const byte ST_MOVE  = 2;
const byte ST_RUN   = 3;
const byte ST_EXP   = 4;
const byte ST_WAIT  = 5;
unsigned long time = 0;

 // initialize core objects
OMCamera     Camera = OMCamera();
OMMotor      Motor  = OMMotor();
OMMoCoNode   Node   = OMMoCoNode(&Serial, device_address, SERIAL_VERSION, (char*) SERIAL_TYPE);
OMComHandler_AT90USB ComMgr = OMComHandler_AT90USB();
    // there are 6 possible states in 
    // our engine (0-5)
OMState      Engine = OMState(6);

int incomingByte = 0;


/* 

 =========================================
 Bluetooth Test Variables
 =========================================
 
*/


AltSoftSerial altSerial;
OMMoCoNode   NodeBlue   = OMMoCoNode(&altSerial, device_address, SERIAL_VERSION, (char*) SERIAL_TYPE);


char inData[255]; // Allocate some space for the string
char inChar1; // Where to store the character read
char inChar2; 
byte index = 0; // Index into array; where to store the character
int a = 50;
int voltage =0;
byte sendBlueFlag = false;
byte sendSerialFlag = false;

byte temp_addr;
byte temp_subaddr;
byte temp_command;
byte temp_bufLen;
byte*temp_buf;


/* 

 =========================================
 Setup and loop functions
 =========================================
 
*/


void setup() {
  
  USBSerial.begin(9600);
  delay(100);
  
  altSerial.begin(9600);
  altSerial.println("Hello World");
  USBSerial.println("HI");
  time = millis();


  pinMode(DEBUG_PIN, OUTPUT);
  pinMode(PBT_PIN, INPUT);
  pinMode(CAM1_SHT_PIN, OUTPUT);
  pinMode(CAM1_FOC_PIN, OUTPUT);
  pinMode(0, OUTPUT);
  digitalWrite(0,HIGH);
  pinMode(42, INPUT);
  pinMode(41, INPUT);
  
    // pull PBT up internally
  digitalWrite(PBT_PIN, HIGH);
  
    // handle reading the PBT pin to see if a factory reset is
    // required
 checkReset();
    
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

 
 
  // Listen for address change
 Node.addressCallback(changeNodeAddr);
 
 NodeBlue.addressCallback(changeNodeAddr);
 
  
  // defaults for motor
 Motor.enable(true);
 Motor.continuous(false);
 Motor.maxStepRate(MOT_DEFAULT_MAX_STEP);
 Motor.maxSpeed(MOT_DEFAULT_MAX_SPD);
 Motor.sleep(true);
 Motor.ms(16);
 
  // enable limit switch handler
// limitSwitch(true);
 
  // startup LED signal
 flasher(DEBUG_PIN, START_FLASH_CNT);

 
  
}





void loop() {
  

   // check to see if we have any commands waiting      
  Node.check();
  /*
  if (sendSerialFlag) {
    USBSerial.print("Before serial send");
    delay(10);
    Node.sendPacket(temp_addr,temp_subaddr,temp_command,temp_bufLen,temp_buf);
    sendSerialFlag = 0;
    USBSerial.println("---- Serial Sent");
  }
  */
  NodeBlue.check();
  /*
  if (sendBlueFlag){
    USBSerial.print("Before bluetooth send");
    NodeBlue.sendPacket(temp_addr,temp_subaddr,temp_command,temp_bufLen,temp_buf);
    sendBlueFlag = 0;
    USBSerial.println("---- Blue Sent");
  }
  */
  /*
  int x = 0;
  x = NodeBlue.getPacket();
  if ( x > 0)
  USBSerial.println(x);
  /*
   while(USBSerial.available() > 0) // Don't read unless
   {
       inChar1 = USBSerial.read(); // Read a character
       delay(10);
       altSerial.print(inChar1);  //print to bluetooth
       USBSerial.print("Sent ");
       USBSerial.println(inChar1);
   }
   while(altSerial.available() > 0) // Don't read unless
   {
       inChar2 = altSerial.read(); // Read a character from bluetooth
      // altSerial.print(inChar2);
      delay(10);
       //digitalWrite(28,HIGH);
   
       altSerial.print(inChar2);  //print to bluetooth
       //digitalWrite(28, LOW);
       inData[index] = inChar2;
       index++;

   }
   */
   if ((millis()-time) > 500)
   {   
     for (int i = 0; i < index; i++)
     {
       //digitalWrite(28, HIGH);
       //USBSerial.print(inData[i]);
       //digitalWrite(28, LOW);
     }
     time = millis();
     //altSerial.print(j);
     voltage=analogRead(42);  
     a=analogRead(41);
     USBSerial.print("Voltage is ");
     USBSerial.print((float)voltage/1023*5*5);
     USBSerial.print(" Current is ");
     USBSerial.println((float)a/1023*5);
     index = 0;
   }
   
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
  Motor.stop();
  running = false;
}



void stopProgram(boolean force_clear) {
              
   // stop/clear program
   
  if( force_clear == true ) {
    run_time     = 0;
    camera_fired = 0;
    mtpc_start   = false;
  }

  running      = false;
  
    // clear out motor moved data
    // and stop motor  
  Motor.clear();
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


  // check for factory reset on startup
void checkReset() {
  
  unsigned long rstTm = millis();
  
  while( digitalRead(PBT_PIN) == LOW ) {
    digitalWrite(DEBUG_PIN, HIGH);
    
    unsigned long newTm = millis();
    
    if( newTm - rstTm > START_RST_TM ) {
        // set eeprom back to defaults
      OMEEPROM::saved(false);
      digitalWrite(DEBUG_PIN, LOW);
        // indicate memory reset by flashing shutter 10 times
      flasher(CAM1_SHT_PIN, START_FLASH_CNT * 2);
      
      break;
    }
    
  }

  digitalWrite(DEBUG_PIN, LOW);
}

void setBlueFlag( byte addr, byte subaddr, byte command, byte bufLen, byte*buf ){
  
  sendBlueFlag = true;
  temp_addr = addr;
  temp_subaddr = subaddr;
  temp_command = command;
  temp_bufLen = bufLen;
  temp_buf = buf;
  
  
}

void setSerialFlag( byte addr, byte subaddr, byte command, byte bufLen, byte*buf ){
  
  sendSerialFlag = true;
  temp_addr = addr;
  temp_subaddr = subaddr;
  temp_command = command;
  temp_bufLen = bufLen;
  temp_buf = buf;
  
  
}

