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

/* 

 =========================================
 MoCoBus Node Command Handling
 nanoMoCo Node Type
 =========================================
 
*/

/* 

  

   --> NOTE: The serial protocol is a BINARY protocol, not an ascii
   protocol.  All data transmitted should be the binary representation.
   
   --> All multi-byte values shall be transmitted in BIG ENDIAN form,
   that is, always start with the MSB and end with the LSB
   
   
   
   MoCoBus Packet Data Definition for nanoMoCo Node:   
   
   Data Section:
   
   Byte0   = Command code (1-254)
   Byte1   = Command data length (0-254 bytes)
   Byte2-x = <data length bytes>
   
   
   --> For each request, a response will be sent indicating success or failure.
   A successful request will respond with a binary value of 1, 0x01.  A 
   failed request will result in a response of 0, 0x00.
   
   **** See the included PDF file with this source distribution for protocol commands. ****

  For all requests that set a value or trigger a command,
  (i.e. not status requests) the response data len will be
  0.  
  
  For all status requests, the response data len will be
  greater than or equal to 1.  
       
*/
    
    
/* Handles Node 1 Commands

  only Node1 goes through this function, determines which node
  to respond to

  */

void serNode1Handler(byte subaddr, byte command, byte*buf) {
  node = 1;
  commandTime = millis();
  USBSerial.print("MocoBus ");
  USBSerial.print("SubAddr: ");
  USBSerial.print(subaddr);
  USBSerial.print(" command: ");
  USBSerial.print(command);
  USBSerial.print(" buf[0]: ");
  USBSerial.print(buf[0], HEX);
  USBSerial.print(" buf[1]: ");
  USBSerial.print(buf[1], HEX);
  USBSerial.print(" buf[2]: ");
  USBSerial.print(buf[2], HEX);
  USBSerial.print(" buf[3]: ");
  USBSerial.print(buf[3], HEX);
  USBSerial.print(" buf[4]: ");
  USBSerial.print(buf[4], HEX);
  USBSerial.print(" time: ");
  USBSerial.println(commandTime);
  USBSerial.println("");
  serCommandHandler(subaddr, command, buf);
}

/* Handles Node 2 Commands

  only NodeBlue goes through this function, determines which node
  to respond to

  */

void serNodeBlueHandler(byte subaddr, byte command, byte*buf) {
  node = 2;
  commandTime = millis();
  USBSerial.print("Bluetooth ");
  USBSerial.print("SubAddr: ");
  USBSerial.print(subaddr);
  USBSerial.print(" command: ");
  USBSerial.print(command);
  USBSerial.print(" buf[0]: ");
  USBSerial.print(buf[0], HEX);
  USBSerial.print(" buf[1]: ");
  USBSerial.print(buf[1], HEX);
  USBSerial.print(" buf[2]: ");
  USBSerial.print(buf[2], HEX);
  USBSerial.print(" buf[3]: ");
  USBSerial.print(buf[3], HEX);
  USBSerial.print(" buf[4]: ");
  USBSerial.print(buf[4], HEX);
  USBSerial.print(" time: ");
  USBSerial.println(commandTime);
  
  serCommandHandler(subaddr, command, buf);
}



/* Handles Node USB Commands

  only NodeUSB goes through this function, determines which node
  to respond to

  */

void serNodeUSBHandler(byte subaddr, byte command, byte*buf) {
  node = 3;
  serCommandHandler(subaddr, command, buf);
}



/* Handles Node Packets not for this device

  only Node goes through this function, repeats the packet to NodeBlue

  */

void serNotUsNode1Handler(byte addr, byte subaddr, byte command, byte bufLen, byte*buf) {

  NodeBlue.sendPacket(addr,subaddr,command,bufLen,buf);
  NodeUSB.sendPacket(addr,subaddr,command,bufLen,buf);
}


/* Handles Node 2 Commands

  only NodeBlue goes through this function, repeats the packet to Node1

  */

void serNotUsNodeBlueHandler(byte addr, byte subaddr, byte command, byte bufLen, byte*buf) {
  

  Node.sendPacket(addr,subaddr,command,bufLen,buf);
}


/* Handles Node USB Commands

  only NodeUSB goes through this function, repeats the packet to Node1

  */

void serNotUsNodeUSBHandler(byte addr, byte subaddr, byte command, byte bufLen, byte*buf) {
  

  Node.sendPacket(addr,subaddr,command,bufLen,buf);
}
    

/* Handle normal commands

  2 = program control commands
  3 = data setting commands
 
  all else generates error
  */
  
void serCommandHandler(byte subaddr, byte command, byte* buf) {
	
	//update the last time a command was received 
	commandTime = millis();

 switch(subaddr) {   
   case 0:

	   // Check for joystick mode and return on non-valid commands if true
	   if (joystick_mode == true && command != 23 && command != 120) {
		   //USBSerial.println("Invalid general command");
		   response(false);
		   return;   
	   }

         // program control
         serMain(command, buf);
         break;
   case 1:
   case 2:
   case 3:

	   // Check for joystick mode and return on non-valid commands if true
	   if (joystick_mode == true && command != 3 && command != 4 && command != 6 && command != 13 && command != 15) {
		   //USBSerial.println("Invalid motor command");
		   response(false);
		   return;
		   
	   }

         //serial motor commands
         serMotor(subaddr, command, buf);
         break;
   case 4:
         //serial camera commands
         serCamera(subaddr, command, buf);
         break;
   default:
         response(false);
         break;
 }

}

 // handle broadcast commands - do not send any
 // responses to a broadcast command!
 
void serBroadcastHandler(byte subaddr, byte command, byte* buf) {
  
  switch(command) {
    case OM_BCAST_START:
      startProgram();
      break;
      
    case OM_BCAST_STOP:
      stopProgram();
      break;
      
    case OM_BCAST_PAUSE:
      pauseProgram();
      break;
	  
	  //resets controller to default address/name, flashes the debug LED 10 times to indicate restart required      
    case OM_BCAST_SET_ADDRESS:
	  if (buf[0] <= 255 && buf[0] >= 2){
		  	device_address = buf[0];
			eepromWrite();
			Node.address(device_address);
			NodeBlue.address(device_address);
			flasher(DEBUG_PIN, 5);	
	  }
      break;
      
    default:
      break;
  }
  
}


/*=========================================
              Main Functions
=========================================== */


void serMain(byte command, byte* input_serial_buffer) {
  
  switch(command) {
    
    //Command 2 starts program  
    case 2:
      startProgram();
      response(true);
      break;
    
    //Command 3 pauses program  
    case 3:
      pauseProgram();
      response(true);
      break;
    
    //Command 4 stops program  
    case 4:
      stopProgram();
      response(true);
      break;
    
    //Command 5 enables or disables the debug LED  
    case 5:

		//debug_led_enable = input_serial_buffer[0];
		// turn off led in case it was on an on cycle
		static uint8_t toggle = false;
		if (!toggle) {
			debugOff();
			toggle = true;
		}
		else {
			debugOn();
			toggle = false;
		}
        
      response(true);
      break;
    
    //Command 6 sets the master timing  
    case 6:
      ComMgr.master(input_serial_buffer[0]);
      response(true);      
      break;
   
    //Command 7 sets the name of the device  
    case 7:
      {  
         memset(device_name, 0, 10);
               
           for(byte i = 0; i <= 10; i++) {
             if( input_serial_buffer[i] != 0 ) 
               device_name[i] = input_serial_buffer[i];
           }
      }             
      
      // save to eeprom
      OMEEPROM::write(EE_NAME, *device_name, 10);
      response(true);
      break;
	  
	//Command 8 sets new address for device
	case 8:
	  if ( input_serial_buffer[0] < 2 || input_serial_buffer[0] > 255) {
		  response(false);
	  } else {
		  device_address= input_serial_buffer[0];
		  Node.address(device_address);
		  NodeBlue.address(device_address);
		  OMEEPROM::write(EE_ADDR, device_address);
		  response(true);
	  }
	  break;
      
    //Command 9 sets a common line for step pulsing  
    case 9:
      if( input_serial_buffer[0] > 2 ) {
        response(false);
      }
      else {
        if( input_serial_buffer[0] == 0 )
          ComMgr.stopWatch();
        else
          ComMgr.watch(input_serial_buffer[0]);
      }
      response(true);
      break;
	  
	//Command 10 send all motors home
	case 10:
		// send a motor home
		motor[0].home();
		motor[1].home();
		motor[2].home();
		startISR();
		response(true);
		break;
		
	//Command 11 set the max step rate of all motors
	case 11:
		maxStepRate(Node.ntoui(input_serial_buffer));
		response(true);
		break;
		
		
	//Command 12 sets limit switch mode (0 - enable on RISING edge, 1 - enable on FALLING edge, 2 - enable on CHANGE edge)
	case 12:
		limitSwitchAttach(input_serial_buffer[0]);
		altSetup();
		response(true);
		break;
		
	//Command 13 sets aux I/O mode
	case 13:
		altConnect(0, input_serial_buffer[0]);
		altConnect(1, input_serial_buffer[1]);
		altSetup();
		response(true);
		break;
		
	//Command 14 sets joystick watchdog flag
	case 14:
	    watchdog = input_serial_buffer[0];
		response(true);
		break;
		
	//Command 15 Set Alt Output Before Shot Delay
	case 15:
		altBeforeDelay = Node.ntoui(input_serial_buffer);
		altSetup();
		response(true);
		break;
		
	//Command 16 Set Alt Output After Shot Delay
	case 16:
		altAfterDelay = Node.ntoui(input_serial_buffer);
		altSetup();
		response(true);
		break;
		
	//Command 17 Set Alt Output Before Shot Time
	case 17:
		altBeforeMs = Node.ntoui(input_serial_buffer);
		altSetup();
		response(true);
		break;

	//Command 18 Set Alt Output After Shot Time
	case 18:
		altAfterMs = Node.ntoui(input_serial_buffer);
		altSetup();
		response(true);
		break;
		
	//Command 19 Set Alt Output Trigger Level
	case 19:
		if (input_serial_buffer[0] == 1){
			altOutTrig = HIGH;
			response(true);
		} else if (input_serial_buffer[0] == 0){
			altOutTrig = LOW;
			response(true);
		} else
			response(false);
		break;
		
	//Command 20 sets the max run time  
	case 20:
		max_time = Node.ntoul(input_serial_buffer);
		response(true);
		break;
	
	//Command 21 set start time delay (input is in seconds)
	case 21:
		start_delay = (Node.ntoul(input_serial_buffer))*1000;//converts the input to milliseconds
		response(true);
		break;

	//Command 22 sets motors' program move mode
	case 22:
		motor[0].planType(input_serial_buffer[0]);
		motor[1].planType(input_serial_buffer[0]);
		motor[2].planType(input_serial_buffer[0]);
		response(true);
		break;

	//Command 23 set joystick mode
	// This will cause the controller to ignore all commands except general commands 23 (set joystick mode) and 120 (query joystick mode),
	// and motor commands 3 (enable motor), 4 (stop now), 6 (set microsteps), 13 (set continuous speed), 15 (execute simple move), and 
	// This is to avoid incorrect commands due to corrupt communications causing runaway motors or controller lockup.
	case 23:
		
		joystick_mode = input_serial_buffer[0];
		
		USBSerial.print("Joystick: ");
		USBSerial.println(joystick_mode);
		
		// Set the speed of all motors to zero when turning on joystick mode to prevent runaway motors
		if (joystick_mode){
			for (byte i = 0; i < MOTOR_COUNT; i++) {
				motor[i].contSpeed(0);
			}
		}
		
		response(true);
		break;
		
	//Command 24 sets the motors' pingPongMode, if enabled it causes the motors to bounce back and forth
	//from the start and stop position until the user stops the program.
	case 24:
		pingPongMode = input_serial_buffer[0];
		response(true);
		break;
        
    
    //*****************MAIN READ COMMANDS********************
    
    //Command 100 read firmware version
    case 100:
      // serial api version
      response( true, (byte) SERIAL_VERSION );
      
      break;
    
    //Command 101 reads run status
    case 101:
      // program run status
      response( true, (byte) running );
      break;
    
    //Command 102 reads run time  
    case 102:
      // current run time
      response(true, run_time);     
      break;
      
    //Command 103 is camera(s) currently exposing?  
    case 103:
      // camera currently exposing
      response( true, (byte) Camera.busy() );
      break;
      
    //Command 104 reads master timing value  
    case 104:
      // timing master    
      response( true, timing_master );
      break;
      
    //Command 105 reads device name 
    case 105:
      // device name
      response(true, (char*)device_name, 10);
      break;
    
	//Command 106 reads max step rate for the motors, can poll any motor        
    case 106:
		response(true, motor[0].maxStepRate());
		break;
		
	//Command 107 reads voltage in
	case 107:
	{
		int voltage=analogRead(VOLTAGE_PIN);
		float converted = (float)voltage/1023*25;
		response(true, converted);
		break;
	}
		
	//Command 108 reads current to the motors
	case 108:
	{
		int current = analogRead(CURRENT_PIN);
		float converted = (float)current/1023*5;
		response(true, converted);
		break;
	}
		
	//Command 109 reads limit switch input edge setting
	case 109:
		response(true, altDirection);
		break;
	
	//Command 110 reads limit switch mode
	case 110:
		temp = 0;
		temp |= altInputs[0]<<8;
		temp |= altInputs[1];
		response(true, temp);
		break;
	
	//Command 111 reads limit switch status high or low (1 or 0)
	case 111:
		temp = 0;
		temp |= digitalRead(AUX_RING)<<8;
		temp |= digitalRead(AUX_TIP);
		response(true, temp);
		break;
	
	//Command 112 reads Alt Output Before Shot Delay Time
	case 112:
		response(true, altBeforeDelay);
		break;
	
	//Command 113 reads Alt Output After Shot Delay Time
	case 113:
		response(true, altAfterDelay);
		break;
	//Command 114 reads Alt Output Before Shot Time
	case 114:
		response(true, altBeforeMs);
		break;
	//Command 115 reads Alt Output After Shot Time
	case 115:
		response(true, altAfterMs);
		break;
		
	//Command 116 reads Alt Output Trigger Level
	case 116:
		response(true, altOutTrig);
		break;

	//Command 117 reads start time delay (seconds)
	case 117:
		response(true, start_delay/1000);
		break;
		
	//Command 118 reads motors' continuous mode setting
	case 118:
		response(true, motor[0].planType());
		break;

	//Command 119 reads whether the controller has been powercycled since last query
	case 119:
		response(true, powerCycled());
		break;

	//Command 120 reads joystick mode setting
	case 120:
		response(true, joystick_mode);
		break;

	//Command 121 reads the ping-pong flag setting
	case 121:
		response(true, pingPongMode);
		break;

	//Command 122 reads the joystick watchdog mode status
	case 122:
		response(true, watchdog);
		break;

	//Commadn 123 reports the percent completion of the current program
	case 123:
		response(true, programPercent());
		break;
	
    //Error    
    default: 
      response(false);
      break;
  }               
}

/*=========================================
              Motor Functions
=========================================== */


void serMotor(byte subaddr, byte command, byte* input_serial_buffer) {
  
  
    switch(command) {

	//Command 2 set the sleep mode of the motor
	case 2:
		motor[subaddr - 1].sleep(input_serial_buffer[0]);
		response(true);
		break;
    
    //Command 3 set motor enable  
    case 3:
      motor[subaddr-1].enable(input_serial_buffer[0]);
      response(true);
      break;

	//Command 4 stops motor now
	case 4:
		// stop motor now
		motor[subaddr - 1].stop();
		response(true);
		break;

	//Command 5 set motor's backlash amount  
	case 5:
		motor[subaddr - 1].backlash(Node.ntoui(input_serial_buffer));
		response(true);
		break;
    
	//Command 6 set the microstep for the motor
	case 6:
		// set motor microstep (1,2,4,8,16)
		tempMS = input_serial_buffer[0];
		motor[subaddr - 1].ms(tempMS);
		OMEEPROM::write(EE_MS_0 + (subaddr - 1) * EE_MOTOR_MEMORY_SPACE, tempMS);
		response(true);
		break;
	
	//Command 7 set the max step speed of the motor
	case 7:
		motor[subaddr - 1].maxSpeed(Node.ntoui(input_serial_buffer));
		response(true);
		break;

	//Command 8 set motor direction  
	case 8:
		motor[subaddr - 1].dir(input_serial_buffer[0]);
		response(true);
		break;

  
    //Command 9 set motor's home limit
    case 9:
      motor[subaddr-1].homeSet();
	  eepromWrite();
      response(true);
      break;	

	//Command 10 set motor's end limit
	case 10:
		tempPos = motor[subaddr - 1].currentPos();
		motor[subaddr - 1].endPos(tempPos);
		OMEEPROM::write(EE_END_0 + (subaddr - 1) * EE_MOTOR_MEMORY_SPACE, tempPos);
		response(true);
		break;

	//Command 11 send motor to home limit
	case 11:
		// Move at the maximum motor speed
		motor[subaddr - 1].ms(4);
		motor[subaddr - 1].contSpeed(MOT_DEFAULT_MAX_STEP);

		// send a motor homE
		motor[subaddr - 1].home();
		startISR();
		response(true);
		break;

	//Command 12 send motor to end limit
	case 12:
		// Move at the maximum motor speed
		motor[subaddr - 1].ms(4);
		motor[subaddr - 1].contSpeed(MOT_DEFAULT_MAX_STEP);

		motor[subaddr - 1].moveToEnd();
		startISR();
		response(true);
		break;
	
    //Command 13 set motor's continous speed 
    case 13:

		// If joystick mode is active and the last speed setting was ~0, automatically start a simple continuous move in the correct direction
		if (joystick_mode){
			
			float old_speed = motor[subaddr - 1].desiredSpeed();
			float new_speed = Node.ntof(input_serial_buffer);
			
			// Set speed
			motor[subaddr - 1].contSpeed(new_speed);

			// Start new move if necessary
			if (abs(old_speed) < 1 && abs(new_speed) > 1){
				byte dir;
				if (new_speed > 1)
					dir = 1;
				else
					dir = 0;

				motor[subaddr - 1].continuous(true);
				motor[subaddr - 1].move(dir, 0);
				startISR();

				USBSerial.println("Auto-starting continuous move");
			}
		}

		// Normal speed change handling
		else
			motor[subaddr - 1].contSpeed(Node.ntof(input_serial_buffer));			
			response(true);

		break;

	//Command 14 sets the acceleration for the motor while in continuous motion
	case 14:
		motor[subaddr - 1].contAccel(Node.ntof(input_serial_buffer));
		response(true);
		break;

	 //Command 15 move motor simple  
	case 15:
	{
			   // set direction
			   byte dir = input_serial_buffer[0];
			   input_serial_buffer++;

			   // if in joystick mode, check whether the speed is currently set to zero and needs to change
			   if (joystick_mode && motor[subaddr - 1].desiredSpeed() < 1.0 && motor[subaddr - 1].desiredSpeed() > - 1.0) {
				   USBSerial.println("Jump-starting the motor!");
				   if (dir == 1)
					   motor[subaddr - 1].contSpeed(10);
				   else
					   motor[subaddr - 1].contSpeed(-10);
			   }

			   // how many steps to take
			   unsigned long steps = Node.ntoul(input_serial_buffer);

			   // move
			   if (steps == 0)
				 motor[subaddr - 1].continuous(true);
				
			   motor[subaddr - 1].move(dir, steps);
			   startISR();

			   response(true);
			   break;
	}

	//Command 16 set program start point
	case 16:
		tempPos = Node.ntol(input_serial_buffer);
		motor[subaddr - 1].startPos(tempPos);
		OMEEPROM::write(EE_START_0 + (subaddr - 1) * EE_MOTOR_MEMORY_SPACE, tempPos);
		response(true);
		break;

	//Command 17 set program stop point
	case 17:
		tempPos = Node.ntol(input_serial_buffer);
		motor[subaddr - 1].stopPos(tempPos);
		OMEEPROM::write(EE_STOP_0 + (subaddr - 1) * EE_MOTOR_MEMORY_SPACE, tempPos);
		response(true);
		break;
   
    //Command 18 set motor's easing mode  
	case 18:
      if( input_serial_buffer[0] == 1 )
        motor[subaddr-1].easing(OM_MOT_LINEAR);
      else if( input_serial_buffer[0] == 2 )
        motor[subaddr-1].easing(OM_MOT_QUAD);
      else if( input_serial_buffer[0] == 3 )
        motor[subaddr-1].easing(OM_MOT_QUADINV);
      else
        response(false);
      response(true);
      break;

	//Command 19 set motor's lead-in shots (number of shots to wait after program start to begin moving)
	case 19:
		motor[subaddr - 1].planLeadIn(Node.ntoi(input_serial_buffer));
		response(true);
		break;

	//Command 20 set shots (SMS) / motor travel time (cont.)
	case 20:
		motor[subaddr - 1].planTravelLength(Node.ntoul(input_serial_buffer));
		response(true);
		break;

	//Command 21 set program acceleration period
	case 21:
		motor[subaddr - 1].planAccelLength(Node.ntoul(input_serial_buffer));
		response(true);
		break;
	
	//Command 22 set program deceleration period
	case 22:
		motor[subaddr - 1].planDecelLength(Node.ntoul(input_serial_buffer));
		response(true);
		break;
      
	//Commnad 23 send motor to program start point
	case 23:
		// Move at the maximum motor speed
		motor[subaddr - 1].ms(4);
		motor[subaddr - 1].contSpeed(MOT_DEFAULT_MAX_STEP);

		motor[subaddr - 1].moveToStart();
		startISR();
		response(true);
		break;
	
	//Commnad 24 send motor to program stop point
	case 24:
		// Move at the maximum motor speed
		motor[subaddr - 1].ms(4);
		motor[subaddr - 1].contSpeed(MOT_DEFAULT_MAX_STEP);

		motor[subaddr - 1].moveToStop();
		startISR();
		response(true);
		break;

	//Command 25 steps forward one planned cycle for SMS
	case 25:
		// step forward one interleaved (sms) plan cycle
	  	
		if( ! motor[subaddr-1].mt_plan ) {
			response(false);
		}
		else {
			// dig this?  We advance one frame by simply turning on autopausing
			// and then playing.  This is a convienence function for clients,
			// rather than forcing them to run both commands.
		  	
			// go ahead and make sure we fire immediately
			camera_tm = millis() - Camera.delay;

			motor[subaddr-1].autoPause = true;
			startProgram();
			response(true);
		}
	  	
		break;
	  	
	//command 26 steps back one sms planed cycle
	case 26:
	// step back one interleaved (sms) plan cycle
	  	
	// return an error if we don't actually have a planned move
		if( ! motor[subaddr-1].mt_plan )
			response(false);
		else {
			if( motor[subaddr-1].planLeadIn() > 0 && camera_fired < motor[subaddr-1].planLeadIn() ) {
				// do not reverse the plan, the motor isn't supposed to move here
			}
			else {
				// rollback one shot in the program
				if( camera_fired > 0 )
					camera_fired--;
			  	
				motor[subaddr-1].planReverse();
				startISR();
			}
		  	
			// need to decrease run time counter
			{
				unsigned long delayTime = ( Camera.delay > (Camera.triggerTime() + Camera.focusTime() + Camera.delayTime()) ) ? Camera.delay : (Camera.triggerTime() + Camera.focusTime() + Camera.delayTime());
			  	
				if( run_time >= delayTime )
					run_time -= delayTime;
				else
					run_time = 0;
			}
			response(true);
		  	
		} // end else (mt_plan
	  	
		break;

	//command 27 reset limits and program positions
	case 27:
		motor[subaddr - 1].homeSet();
		motor[subaddr - 1].endPos(0);
		motor[subaddr - 1].startPos(0);
		motor[subaddr - 1].stopPos(0);
		eepromWrite();
		response(true);
		break;

	//command 28 automatically sets the motor to the highest resolution microstepping possible given program parameters
	//The command will respond with the value that is selected or with 0 if a program is in progress or the selected motor is running.
	case 28:

		// Don't change the microstep value if the motor or program is running
		if (!running && !motor[subaddr - 1].running()) {
			
			// The microstepping cutoff values below are in 16th steps
			const int QUARTER_CUTOFF = 10000;
			const int EIGHTH_CUTOFF = 5000;
			float comparison_speed;

			// For time lapse SMS mode
			if (motor[subaddr - 1].planType() == 0) {

				const float MILLIS_PER_SECOND = 1000.0;
				
				// Max time in seconds
				float max_time_per_move = (float)(Camera.delay - Camera.delayTime - Camera.triggerTime() - Camera.focusTime()) / MILLIS_PER_SECOND;

				
				// The "topSpeed" variable in SMS mode is actually the number of steps per move during the constant speed segment
				float steps_per_move = motor[subaddr - 1].getTopSpeed();

				comparison_speed = steps_per_move / (float) max_time_per_move;

			}

			// For time lapse continuous mode and video continuous mode
			else if (motor[subaddr - 1].planType() == 1 || motor[subaddr - 1].planType() == 2) {
				comparison_speed = motor[subaddr - 1].getTopSpeed();
			}

			// Check the comparison speed against the cutoff values and select the appropriate microstepping setting
			if (comparison_speed >= QUARTER_CUTOFF)
				motor[subaddr - 1].ms(4);
			else if (comparison_speed < QUARTER_CUTOFF && comparison_speed > EIGHTH_CUTOFF)
				motor[subaddr - 1].ms(8);
			else
				motor[subaddr - 1].ms(16);

			// Report back the microstep value that was auto-selected
			response(true, motor[subaddr - 1].ms());

		}

		// If the motor or program is running, report back 0 to indicate that the auto-set routine was not completed
		else
			response(true, 0);

		break;

	//Command 228 set program start point here
	case 228:
		tempPos = motor[subaddr - 1].currentPos();
		motor[subaddr - 1].startPos(tempPos);
		OMEEPROM::write(EE_START_0 + (subaddr - 1) * EE_MOTOR_MEMORY_SPACE, tempPos);
		response(true);
		break;

	//Command 229 set program stop point here
	case 229:
		tempPos = motor[subaddr - 1].currentPos();
		motor[subaddr - 1].stopPos(tempPos);
		OMEEPROM::write(EE_STOP_0 + (subaddr - 1) * EE_MOTOR_MEMORY_SPACE, tempPos);
		response(true);
		break;
		
	

      
    

    
    //*****************MOTOR READ COMMANDS********************
    
    //Command 100 reads motor enable status
    case 100:
      response( true, (byte) motor[subaddr-1].enable() );
      break;

	//Command 101 reads the backlash amount for the motor 
	case 101:
		response(true, motor[subaddr - 1].backlash());
		break;

	//Command 102 reads the microstep setting of the motor
	case 102:
		response(true, motor[subaddr - 1].ms());
		break;

	//Command 103 reads motor's direction
	case 103:
		response(true, (byte)motor[subaddr - 1].dir());
		break;

	//Command 104 reads max step speed of the motor
	case 104:
		response(true, motor[subaddr - 1].maxSpeed());
		break;

	//Command 105 reads end limit position
	case 105:
		response(true, motor[subaddr - 1].endPos());
		break;

	//Command 106 reads the motor's current position (steps from home limit)
	case 106:
		response(true, motor[subaddr - 1].currentPos());
		break;

	//Command 107 reads whether the motor is currently running
	case 107:
		response(true, motor[subaddr - 1].running());
		break;

	//Command 108 reads the continuous speed for the motor
	case 108:
		response(true, motor[subaddr - 1].contSpeed());
		break;

	//Command 109 reads the accel/decel value continuous motion
	case 109:
		response(true, motor[subaddr - 1].contAccel());
		break;

	//Command 110 reads the easing algorithm
	case 110:
		response(true, motor[subaddr - 1].easing());
		break;

	//Command 111 reads the program start point position
	case 111:
		response(true, motor[subaddr - 1].startPos());
		break;

	//Command 112 reads the program stop point position
	case 112:
		response(true, motor[subaddr - 1].stopPos());
		break;

	//Command 113 reads the program shots (SMS) / motor travel time (cont.)
	case 113:
		response(true, motor[subaddr - 1].planTravelLength());
		break;	

	//Command 114 reads the motor's lead-in shots count
	case 114:
		response(true, motor[subaddr - 1].planLeadIn());
		break;

	//Command 115 reads the program acceleration rate
	case 115:
		response(true, motor[subaddr - 1].planAccelLength());
		break;

	//Command 116 reads the program deceleration rate
	case 116:
		response(true, motor[subaddr - 1].planDecelLength());
		break;
   
    //Error    
    default: 
      response(false);
      break;
  } 
  
}

/*=========================================
              Camera Functions
=========================================== */


void serCamera(byte subaddr, byte command, byte* input_serial_buffer) {
  
  switch(command) {
    
    //Command 2 set camera enable  
    case 2:
      Camera.enable = input_serial_buffer[0];
      response(true);
      break;
    
    //Command 3 expose camera now 
    case 3:
      Camera.expose();
      response(true);
      break;
    
    //Command 4 set camera's exposure time  
    case 4:
      Camera.triggerTime( Node.ntoul(input_serial_buffer) );
      response(true);
      break;
      
    //Command 5 set camera's focus time
    case 5:
      Camera.focusTime( Node.ntoui(input_serial_buffer) ); 
      response(true);
      break;
    
    //Command 6 set camera's max shots 
    case 6:
      Camera.maxShots  = Node.ntoui(input_serial_buffer);
      response(true);
      break;

    //Command 7 set camera's exposure delay
    case 7:
      Camera.delayTime( Node.ntoui(input_serial_buffer) );
      response(true);
      break;
   
    //Command 8 set camera's focus w shutter  
    case 8:
      Camera.exposureFocus((uint8_t) input_serial_buffer[0]);
	  response(true);
      break;
      
    //Command 9 repeat cycles
    case 9:
      Camera.repeat = input_serial_buffer[0];
      response(true);
      break;
      
    //Command 10 set camera's interval  
    case 10:
      Camera.delay = Node.ntoul(input_serial_buffer);
      response(true);
      break;
    
    
    //*****************CAMERA READ COMMANDS********************
    
    //Command 100 gets camera's enable status
    case 100:
      response( true, (byte) Camera.enable );
      break;
    
    //Command 101 gets if it's exposing now or not
    case 101:
      response( true, (byte) Camera.busy() );
      break;
    
    //Command 102 gets the camera's exposure time
    case 102:
      response(true, Camera.triggerTime());   
      break;
      
    //Command 103 gets the camera's focus time
    case 103:
      response( true, Camera.focusTime() );
      break;
      
    //Command 104 gets the camera's max shots
    case 104:
      response( true, Camera.maxShots );
      break;
      
    //Command 105 gets the camera's exposure delay
    case 105:
      response(true, Camera.delayTime());
      break;
      
    //Command 106 gets the focus with shutter status
    case 106:
      response(true, Camera.exposureFocus());
      break;
      
    //Command 107 gets the MUP state
    case 107:
      response(true, Camera.repeat);
      break;
      
    //Command 108 gets the camera's interval time
    case 108:
      response(true, Camera.delay);
      break;

	//Command 109 gets the number of shots fired 
	case 109:
		response(true, camera_fired);
		break;
      
            
    //Error    
    default: 
      response(false);
      break;
  }
  
}


           
             
           
             


void serialComplexMove(byte subaddr, byte* buf) {
   byte dir = buf[0];
   buf++;
   
   unsigned long dist  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long arrive  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long accel  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long decel  = Node.ntoul(buf);

   motor[subaddr-1].move(dir, dist, arrive, accel, decel); 
}


/*=========================================
          Node Response Functions
===========================================*/

void response(uint8_t p_stat){
	switch(node){
		case 3:
			NodeUSB.response(p_stat);
			break;
		case 2:
			NodeBlue.response(p_stat); 
			break;
		case 1:
			Node.response(p_stat); 
			break;
		default:
			break;
	}
    
} 

void response(uint8_t p_stat, uint8_t p_resp){
	switch(node){
		case 3:
			NodeUSB.response(p_stat, p_resp);
			break;
		case 2:
			NodeBlue.response(p_stat, p_resp);
			break;
		case 1:
			Node.response(p_stat, p_resp); 
			break;
		default:
			break;
	}
}

void response(uint8_t p_stat, unsigned int p_resp){
	switch(node){
		case 3:
			NodeUSB.response(p_stat, p_resp);
			break;
		case 2:
			NodeBlue.response(p_stat, p_resp);
			break;
		case 1:
			Node.response(p_stat, p_resp);
			break;
		default:
			break;
	}
}

void response(uint8_t p_stat, int p_resp){
	switch(node){
		case 3:
			NodeUSB.response(p_stat, p_resp);
			break;
		case 2:
			NodeBlue.response(p_stat, p_resp);
			break;
		case 1:
			Node.response(p_stat, p_resp);
			break;
		default:
			break;
	}
}

void response(uint8_t p_stat, unsigned long p_resp){
	switch(node){
		case 3:
			NodeUSB.response(p_stat, p_resp);
			break;
		case 2:
			NodeBlue.response(p_stat, p_resp);
			break;
		case 1:
			Node.response(p_stat, p_resp);
			break;
		default:
			break;
	}
}

void response(uint8_t p_stat, long p_resp){
	switch(node){
		case 3:
			NodeUSB.response(p_stat, p_resp);
			break;
		case 2:
			NodeBlue.response(p_stat, p_resp);
			break;
		case 1:
			Node.response(p_stat, p_resp);
			break;
		default:
			break;
	}
}

void response(uint8_t p_stat, float p_resp){
	switch(node){
		case 3:
			NodeUSB.response(p_stat, p_resp);
			break;
		case 2:
			NodeBlue.response(p_stat, p_resp);
			break;
		case 1:
			Node.response(p_stat, p_resp);
			break;
		default:
			break;
	}
}

void response(uint8_t p_stat, char* p_resp, int p_len){
	
	switch(node){
		case 3:
			NodeUSB.response(p_stat, p_resp, p_len);
			break;
		case 2:
			NodeBlue.response(p_stat, p_resp, p_len);  
			break;
		case 1:
			Node.response(p_stat, p_resp, p_len);
			break;
		default:
			break;
	}
}





