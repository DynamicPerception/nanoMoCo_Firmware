// NMX_Commands.h

#ifndef _NMX_COMMANDs_h
#define _NMX_COMMANDs_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

/* Command format:

65,000

Query		Group		Sub-group	Command #

0 or 1		0 - 9		0-9			0-99

Group / sub-group codes:

0 -- General controller settings

	0 -- Controller settings

		** Command **

			00000 - Set device address
			00001 - Set Stored Name
			00002 - Timing Master			
			00003 - Set Common Line for Step Pulsing

		** Query **

			10000 - N/A
			10001 - Device name
			10002 - Timing master value
			10002 - N/A
			10003 - N/A
			10004 - Firmware Version
			10005 - Voltage Reading
			10006 - Controller Power Cycle

	1 -- Alt input commands

		** Command **

			00100 - Alt Input Edge (RISING, FALLING, or CHANGE)
			00101 - Alt I/O Mode
			00102 - Alt Output Before Shot Delay Time
			00103 - Alt Output After Shot Delay Time
			00104 - Alt Output Before Shot Time
			00105 - Alt Output After Shot Time
			00106 - Alt Output Trigger level

		** Query **

			10100 - Alt Input Edge (RISING, FALLING, or CHANGE)
			10101 - Alt I/O Mode
			10102 - Alt Output Before Shot Delay Time
			10103 - Alt Output After Shot Delay Time
			10104 - Alt Output Before Shot Time
			10105 - Alt Output After Shot Time
			10106 - Alt Output Trigger level
			10107 - Limit Switch High/Low Status
		

1 -- Motor commands

	0 -- Motor 0
	1 -- Motor 1
	2 -- Motor 2

		** Command **

			** Basic settings **
			
			01000 - Motor Enable
			01001 - Motor Sleep
			01002 - Set Backlash Steps
			01003 - Set Microstep Value
			01004 - Set Motor Max Step Speed
			
			** Hard limits **

			01010 - Set Home Limit Here
			01011 - Set End Limit Here
			01012 - Send Motor to Home Limit
			01013 - Send Motor to End Limit

			** Simple motor control **

			01020 - Stop Motor Now
			01021 - Set Direction
			01022 - Set Continuous Speed
			01023 - Set Motor Continuous Motion Accel/Decel Rate
			01024 - Execute Simple Motor Move
					 
			** Program limits ** 

			01030 - Set Start Here
			01031 - Set Stop Here
			01032 - Set Program Start point
			01033 - Set Program Stop point
			01034 - Send Motor to Program Start Point
			01035 - Send Motor to Program Stop Point
			01036 - Reset Limits and Program Start/Stop Positions

			** Program movement ** 
			
			01040 - Set Easing (Ramping) Mode
			01041 - Set Lead-In Shots / Time
			01042 - Set Program Accel
			01043 - Set Travel Shots(SMS) / Travel Time (Cont.)
			01044 - Set Program Decel
			01045 - Set lead-out shots / time			
			01046 - Auto Set Program Microsteps
		

		** Query **

			** Basic settings **

			11000 - Motor Enable
			11001 - Check Motor Sleep State
			11002 - Backlash Steps
			11003 - Microstep Value
			11004 - Motor Max Step Speed

			** Hard limits **

			11000 - N/A
			11011 - End Limit Position
			

			** Simple motor control **

			Current Motor Position
			Motor Running
			Direction
			Continuous Speed
			Motor Continuous Motion Accel/Decel Rate

			Easing (Ramping) Mode

			** Program limits **

			Program Start point
			Program Stop point
			Travel Shots(SMS) / Travel Time (Cont.)
			Lead-In Shots / Time
			Program Accel
			Program Decel
			
			** Program movement **


			Check Motor Speed Limit
			Lead-Out Shots / Time	
					

	4 -- All motors

		** Command **
		
			Return Home All Motors
			Motors Max Step Rate
			Set Joystick Mode
			Set Joystick Watchdog
			Send all Motors to Program Start
			Set Program Start point
			Set Program Stop point

		** Query **

			Current to Motors
			Motors Max Step Rate
			Start Program Delay
			SMS / Continuous Program Mode
			Joystick Mode
			Joystick Watchdog Mode Status
			Check Motor Attachment
			Check All Motor Run Status
			Check Sleep State of All Motors

	5 -- Motor program settings

2 -- Camera commands

	0 -- General control

	1 -- Camera program settings

		** Command **

			** Simple control **
			
			Camera Enable
			Expose Now
			
			** Program parameters **

			Interval
			Trigger Time
			Focus Time
			Max Shots
			Exposure Delay
			Focus w Shutter
			Mirror Up (Repeat Shot)
			
			** Test Mode **

			Camera Test Mode

		** Query **

			** Program Status ** 
			
			Current Shots
			Currently Exposing
			
			** Program parameters **

			Camera Enable
			Interval Time
			Trigger Time
			Focus Time
			Max Shots
			Exposure Delay
			Focus w Shutter
			Mirror Up (Repeat Shot)
			
			** Test Mode **

			Camera Test Mode			

3 -- General program commands

	0 -- All 

		** Command **

			Start
			Pause
			Stop
			Max Program Run Time
			Start Program Delay
			Set SMS / Continuous Program Mode
			Set Ping-Pong Flag
			Set Frames/Second Flag

		** Query **

			Run Status
			Run Time
			Start Program Delay
			SMS / Continuous Program Mode
			Ping-Pong Flag
			Program % Complete
			Total Program Run Time	
			Program Complete?	
			Check Frames/Second Flag	
			Validate Program
			
	
4 -- Broadcast commands
	
	0 -- All

		** Command**

			Start
			Stop
			Pause
			Assign Address

5 -- Mobile app specific commands

6-- Undefined

7 -- Undefined

8 -- Undefined

9 -- Debug commands

	0 -- All 

		** Command ** 

			00001 - Debug LED
			COM_OUT
			STEPS
			MOTOR
			GEN_SER
			FUNCT
			CONFIRM

For example, setting the constant speed for motor 1 might be command 01113,
while the command for querying the constant speed for motor 1 might be command 11113.

*/

class NMX_CommandsClass
{
 public:
	void init();	

private:


};

extern NMX_CommandsClass NMX_Commands;

#endif

