// NMX_Commands.h

#ifndef _NMX_COMMANDS_H
#define _NMX_COMMANDS_H

/* Command format:

0-65,535

Query		Group		Sub-group	Command #

0 or 1		0 - 9		0-9			0-99

Group / sub-group codes:

0 -- GEN -- General settings
1 -- MOT -- Motor settings
2 -- CAM -- Camera settings
3 -- PRO -- Program settings
4 -- BDC -- Broadcast commands
5 -- MAC -- Mobile app-specific commands
6 -- Undefined
7 -- Undefined
8 -- Undefined
9 -- DBG -- Debug commands

0 -- General settings

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

			11010 - N/A
			11011 - End Limit Position
			
			** Simple motor control **

			11020 - Motor Running
			11021 - Direction
			11022 - Continuous Speed
			11023 - Motor Continuous Motion Accel/Decel Rate
			11024 - N/A
			11025 - Current Motor Position			

			** Program limits **

			11030 - Program Start point
			11031 - Program Stop point
			11032 - N/A
			11033 - N/A
			11034 - N/A
			11035 - N/A
			11036 - N/A			
			
			** Program movement **

			11040 - Easing (Ramping) Mode
			11041 - Lead-In Shots / Time
			11042 - Program Accel
			11043 - Travel Shots(SMS) / Travel Time (Cont.)
			11044 - Program Decel
			11045 - Lead-Out Shots / Time	
			11046 - Validate Motor Speed Limit
					

	3 -- All motors

		** Command **
		
			01300 - Return Home All Motors
			01301 - Motors Max Step Rate
			01302 - Set Joystick Mode
			01303 - Set Joystick Watchdog
			01304 - Set Program Start point
			01305 - Set Program Stop point
			01306 - Send all Motors to Program Start

		** Query **

			11300 - N/A
			11301 - Motors Max Step Rate
			01302 - Joystick Mode
			01303 - Joystick Watchdog Mode Status
			11304 - N/A
			11305 - N/A
			11306 - N/A
			11307 - Start Program Delay
			11308 - SMS / Continuous Program Mode
			11309 - Current to Motors
			11310 - Check Motor Attachment
			11311 - Check All Motor Run Status
			11312 - Check Sleep State of All Motors

2 -- Camera commands

	0 -- General control

		** Command **

			** Simple control **
			
			02000 - Camera Enable
			02001 - Expose Now
			
			** Program parameters **

			02010 - Interval
			02011 - Trigger Time
			02012 - Focus Time
			02013 - Exposure Delay
			02014 - Max Shots
			02015 - Focus w Shutter
			02016 - Mirror Up (Repeat Shot)
			
			** Test Mode **

			02020 - Camera Test Mode

		** Query **

			** Simple Control **
			
			12000 - Camera Enable
			12001 - Currently Exposing
			
			** Program parameters **
					
			12010 - Interval Time
			12011 - Trigger Time
			12012 - Focus Time
			12013 - Exposure Delay
			12014 - Max Shots
			12015 - Focus w Shutter
			12016 - Mirror Up (Repeat Shot)
			12017 - Current Shots
			
			** Test Mode **

			12020 - Camera Test Mode						
			

3 -- General program commands

	0 -- All 

		** Command **

			03000 - Start
			03001 - Pause
			03002 - Stop
			03003 - Max Program Run Time
			03004 - Start Program Delay
			03005 - Set SMS / Continuous Program Mode
			03006 - Set Ping-Pong Flag
			03007 - Set Frames/Second Flag

		** Query **

			13000 - Run Status
			13001 - N/A
			13002 - N/A
			13003 - Elapsed Run Time
			13004 - Start Program Delay
			13005 - SMS / Continuous Program Mode
			13006 - Ping-Pong Flag
			13007 - Check Frames/Second Flag	
			13008 - Program % Complete
			13009 - Total Program Run Time	
			13010 - Validate Program
			13011 - Program Complete?

	
4 -- Broadcast commands
	
	0 -- All

		** Command**

			04000 - Start
			04001 - Stop
			04002 - Pause
			04003 - Assign Address

5 -- Mobile app specific commands

6-- Undefined

7 -- Undefined

8 -- Undefined

9 -- Debug commands

	0 -- All 

		** Command ** 

			09000 - Debug LED
			09001 - COM_OUT
			09002 - STEPS
			09003 - MOTOR
			09004 - GEN_SER
			09005 - FUNCT
			09006 - CONFIRM

For example, setting the constant speed for motor 1 might be command 01113,
while the command for querying the constant speed for motor 1 might be command 11113.

*/

struct NMX_General_Commands {

	/********************************

		0 -- General settings

			0 -- Controller settings

	**********************************/

	/** Command **/

		const unsigned int
									//  00000	Code unused
		ADDR			= 00001,	//	00001 	Set device address
		NAME			= 00002,	//	00002 	Set Stored Name
		TIMING			= 00003,	//	00003 	Timing Master
		COMMON_LINE		= 00004,	//	00004 	Set Common Line for Step Pulsing

	/**Query**/

									//	10000 	 N/A
		q_NAME			= 10001,	//	10001 	 Device name
		q_TIMING		= 10002,	//	10002 	 Timing master value
									//	10002 	 N/A
									//	10003 	 N/A
		q_VERSION		= 10004,	//	10004 	 Firmware Version
		q_VOLTAGE		= 10005,	//	10005 	 Voltage Reading
		q_POWERCYCLE	= 10006;	//	10006 	 Controller Power Cycle

};


struct NMX_Motor_Commands {

	/********************************

		1 -- Motor settings

			0-2 -- Individual Motors

	**********************************/

	/** Command **/

		/** Basic settings **/

	const unsigned int

		ENABLE[MOTOR_COUNT]			= { 01000, 01100, 01200 },		//	01000 	 Motor Enable
		SLEEP[MOTOR_COUNT]			= { 01001, 01101, 01201 },		//	01001 	 Motor Sleep
		BACKLASH[MOTOR_COUNT]		= { 01002, 01102, 01202 },		//	01002 	 Set Backlash Steps
		MICROSTEPS[MOTOR_COUNT]		= { 01003, 01103, 01203 },		//	01003 	 Set Microstep Value
		MAX_SPEED[MOTOR_COUNT]		= { 01004, 01104, 01204 },		//	01004 	 Set Motor Max Step Speed

		/** Hard limits **/

		SET_HOME[MOTOR_COUNT]		= { 01010, 01110, 01210 },		//	01010 	 Set Home Limit Here
		SET_END[MOTOR_COUNT]		= { 01011, 01111, 01211 },		//	01011 	 Set End Limit Here
		SEND_HOME[MOTOR_COUNT]		= { 01012, 01112, 01212 },		//	01012 	 Send Motor to Home Limit
		SEND_END[MOTOR_COUNT]		= { 01013, 01113, 01213 },		//	01013 	 Send Motor to End Limit

		/** Simple motor control **/

		STOP[MOTOR_COUNT]			= { 01020, 01120, 01220 },		//	01020 	 Stop Motor Now
		DIR[MOTOR_COUNT]			= { 01021, 01121, 01221 },		//	01021 	 Set Direction
		CONST_SPEED[MOTOR_COUNT]	= { 01021, 01121, 01221 },		//	01022 	 Set Continuous Speed
		CONST_ACCEL[MOTOR_COUNT]	= { 01022, 01122, 01222 },		//	01023 	 Set Motor Continuous Motion Accel/Decel Rate
		MOVE_SIMPLE[MOTOR_COUNT]	= { 01023, 01123, 01223 },		//	01024 	 Execute Simple Motor Move

		/** Program limits **/

		SET_START_HERE[MOTOR_COUNT]	= { 01030, 01130, 01230 },		//	01030 	 Set Start Here
		SET_STOP_HERE[MOTOR_COUNT]	= { 01031, 01131, 01231 },		//	01031 	 Set Stop Here
		SET_START[MOTOR_COUNT]		= { 01032, 01132, 01232 },		//	01032 	 Set Program Start point
		SET_STOP[MOTOR_COUNT]		= { 01033, 01133, 01233 },		//	01033 	 Set Program Stop point
		SEND_START[MOTOR_COUNT]		= { 01034, 01134, 01234 },		//	01034 	 Send Motor to Program Start Point
		SEND_STOP[MOTOR_COUNT]		= { 01035, 01135, 01235 },		//	01035 	 Send Motor to Program Stop Point
		RESET_START_STOP[MOTOR_COUNT] = { 01036, 01136, 01236 },	//	01036 	 Reset Limits and Program Start/Stop Positions

		/** Program movement **/

		EASING[MOTOR_COUNT]			= { 01040, 01140, 01240 },		//	01040 	 Set Easing (Ramping) Mode
		LEAD_IN[MOTOR_COUNT]		= { 01041, 01141, 01241 },		//	01041 	 Set Lead-In Shots / Time
		ACCEL_PROG[MOTOR_COUNT]		= { 01042, 01142, 01242 },		//	01042 	 Set Program Accel
		TRAVEL_TIME[MOTOR_COUNT]	= { 01043, 01143, 01243 },		//	01043 	 Set Travel Shots(SMS) / Travel Time (Cont.)
		DECEL_PROG[MOTOR_COUNT]		= { 01044, 01144, 01244 },		//	01044 	 Set Program Decel
		LEAD_OUT[MOTOR_COUNT]		= { 01045, 01145, 01245 },		//	01045 	 Set lead-out shots / time
		AUTO_MS[MOTOR_COUNT]		= { 01046, 01146, 01246 },		//	01046 	 Auto Set Program Microsteps


		/** Query**/

		/** Basic settings **/

		q_ENABLE[MOTOR_COUNT]		= { 11000, 11100, 11200 },		//	11000 	 Motor Enable
		q_SLEEP[MOTOR_COUNT]		= { 11001, 11101, 11201 },		//	11001 	 Check Motor Sleep State
		q_BACKLASH[MOTOR_COUNT]		= { 11002, 11102, 11202 },		//	11002 	 Backlash Steps
		q_MICROSTEPS[MOTOR_COUNT]	= { 11003, 11103, 11203 },		//	11003 	 Microstep Value
		q_MAX_SPEED[MOTOR_COUNT]	= { 11004, 11104, 11204 },		//	11004 	 Motor Max Step Speed

		/** Hard limits **/

																	//	11010 	 N/A
		q_END[MOTOR_COUNT]			= { 11011, 11111, 11211 },		//	11011 	 End Limit Position

		/** Simple motor control **/

		q_RUNNING[MOTOR_COUNT]		= { 11020, 11120, 11220 },		//	11020 	 Motor Running
		q_DIR[MOTOR_COUNT]			= { 11021, 11121, 11221 },		//	11021 	 Direction
		q_CONST_SPEED[MOTOR_COUNT]	= { 11022, 11122, 11222 },		//	11022 	 Continuous Speed
		q_CONST_ACCEL[MOTOR_COUNT]	= { 11023, 11123, 11223 },		//	11023 	 Motor Continuous Motion Accel/Decel Rate
																	//	11024 	 N/A
		q_POS[MOTOR_COUNT]			= { 11025, 11125, 11225 },		//	11025 	 Current Motor Position

		/** Program limits **/

		q_START[MOTOR_COUNT]		= { 11030, 11130, 11230 },		//	11030 	 Program Start point
		q_STOP[MOTOR_COUNT]			= { 11031, 11131, 11231 },		//	11031 	 Program Stop point
																

		/** Program movement **/

		q_EASING[MOTOR_COUNT]		= { 11040, 11140, 11240 },		//	11040 	 Easing (Ramping) Mode
		q_LEAD_IN[MOTOR_COUNT]		= { 11041, 11141, 11241 },		//	11041 	 Lead-In Shots / Time
		q_ACCEL_PROG[MOTOR_COUNT]	= { 11042, 11142, 11242 },		//	11042 	 Program Accel
		q_TRAVEL_TIME[MOTOR_COUNT]	= { 11043, 11143, 11243 },		//	11043 	 Travel Shots(SMS) / Travel Time (Cont.)
		q_DECEL_PROG[MOTOR_COUNT]	= { 11044, 11144, 11244 },		//	11044 	 Program Decel
		q_LEAD_OUT[MOTOR_COUNT]		= { 11045, 11145, 11245 },		//	11045 	 Lead-Out Shots / Time
		q_VALIDATE[MOTOR_COUNT]		= { 11046, 11146, 11246 },		//	11046 	 Validate Motor Speed Limit


	/********************************

		1 -- Motor settings

			3 -- All Motors

	**********************************/

	/** Command **/

		SEND_ALL_HOME		= 01300,			//	01300 	 Return Home All Motors
		MAX_STEP_RATE		= 01301,			//	01301 	 Motors Max Step Rate
		JOYSTICK			= 01302,			//	01302 	 Set Joystick Mode
		WATCHDOG			= 01303,			//	01303 	 Set Joystick Watchdog
		SET_START_HERE		= 01304,			//	01304 	 Set Program Start point
		SET_STOP_HERE		= 01305,			//	01305 	 Set Program Stop point
		SEND_ALL_START		= 01306,			//	01306 	 Send all Motors to Program Start

	/** Query**/

												//	11300 	 N/A
		q_MAX_STEP_RATE		= 11301,			//	11301 	 Motors Max Step Rate
		q_JOYSTICK			= 11302,			//	01302 	 Joystick Mode
		q_WATCHDOG			= 11303;			//	01303 	 Joystick Watchdog Mode Status

};

struct NMX_Camera_Commands {

	/********************************

		2 -- Camera settings

			0 -- All commands

	**********************************/


	/** Control **/

		/** Simple control **/

		const unsigned int

		ENABLE,				//	02000 	 Camera Enable
		EXPOSE,				//	02001 	 Expose Now

		/** Program parameters **/

		INTERVAL,			//	02010 	 Interval
		TRIGGER,			//	02011 	 Trigger Time
		FOCUS,				//	02012 	 Focus Time
		DELAY,				//	02013 	 Exposure Delay
		MAX_SHOTS,			//	02014 	 Max Shots
		FOCUS_W_SHUTTER,	//	02015 	 Focus w Shutter
		MUP,				//	02016 	 Mirror Up (Repeat Shot)

		/** Test Mode **/

		TEST_MODE,			//	02020 	 Camera Test Mode

	/** Query **/

		/** Simple Control **/

		q_ENABLE,			//	12000 	 Camera Enable
		Q_EXPOSE,			//	12001 	 Currently Exposing

		/** Program parameters **/

		q_INTERVAL,			//	12010 	 Interval Time
		q_TRIGGER,			//	12011 	 Trigger Time
		q_FOCUS,			//	12012 	 Focus Time
		q_DELAY,			//	12013 	 Exposure Delay
		q_MAX_SHOTS,		//	12014 	 Max Shots
		q_FOCUS_W_SHUTTER,	//	12015 	 Focus w Shutter
		q_MUP,				//	12016 	 Mirror Up (Repeat Shot)
		q_CURRENT_SHOTS,	//	12017 	 Current Shots

		/** Test Mode **/

		q_TEST_MODE;		//	12020 	 Camera Test Mode
};

struct NMX_Program_Commands {

	/********************************

		3 -- Program settings

			0 -- All commands

	**********************************/

	/** Command **/

		const unsigned int

		START,				//	03000 	 Start
		PAUSE,				//	03001 	 Pause
		STOP,				//	03002 	 Stop
		MAX_RUN_TIME,		//	03003 	 Max Program Run Time
		START_DELAY,		//	03004 	 Start Program Delay
		MODE,				//	03005 	 Set SMS / Continuous Program Mode
		PING_PONG,			//	03006 	 Set Ping-Pong Flag
		FPS,				//	03007 	 Set Frames/Second Flag


	/** Query **/

		q_STATUS,			//	13000 	 Run Status
							//	13001 	 N/A
							//	13002 	 N/A
		q_RUN_TIME,			//	13003 	 Elapsed Run Time
		q_START_DELAY,		//	13004 	 Start Program Delay
		q_MODE,				//	13005 	 SMS / Continuous Program Mode
		q_PING_PONG,		//	13006 	 Ping-Pong Flag
		q_FPS,				//	13007 	 Check Frames/Second Flag
		q_PERCENT_COMPLETE,	//	13008 	 Program % Complete
		q_TOTAL_RUN_TIME,	//	13009 	 Total Program Run Time
		q_VALIDATE,			//	13010 	 Validate Program
		q_COMPLETE;			//	13011 	 Program Complete?

};


struct NMX_Broadcast_Commands {

	/********************************

		4 -- Broadcast commands

			0 -- All commands

	**********************************/

	/** Commands **/
		const unsigned int

		START,				//	04000 	 Start
		PAUSE,				//  04001	 Pause
		STOP,				//	04002 	 Stop
		ADDR;				//	04003 	 Assign Address

};

struct NMX_Aux_Commands {


	/********************************

		5 -- Aux Commands

			0 -- All Commands

	**********************************/

	/** Command **/

	const unsigned int

		INPUT_EDGE			= 05000,		//	05000 	 Alt Input Edge (RISING, FALLING, or CHANGE)
		MODE				= 05001,		//	05001 	 Alt I/O Mode
		B4_SHOT_DELAY		= 05002,		//	05002 	 Alt Output Before Shot Delay Time
		AFT_SHOT_DELAY		= 05003,		//	05003 	 Alt Output After Shot Delay Time
		B4_SHOT_TIME		= 05004,		//	05004 	 Alt Output Before Shot Time
		AFT_SHOT_TIME		= 05005,		//	05005 	 Alt Output After Shot Time
		TRIG_LEVEL			= 05006, 		//	05006 	 Alt Output Trigger level


	/** Query**/

		q_INPUT_EDGE		= 15000,		//	15000 	 Alt Input Edge (RISING, FALLING, or CHANGE)
		q_MODE				= 15001,		//	15001 	 Alt I/O Mode
		q_B4_SHOT_DELAY		= 15002,		//	15002 	 Alt Output Before Shot Delay Time
		q_AFT_SHOT_DELAY	= 15003,		//	15003 	 Alt Output After Shot Delay Time
		q_B4_SHOT_TIME		= 15004,		//	15004 	 Alt Output Before Shot Time
		q_AFT_SHOT_TIME		= 15005,		//	15005 	 Alt Output After Shot Time
		q_TRIG_LEVEL		= 15006, 		//	15006 	 Alt Output Trigger level
		q_SWITCH_STATUS		= 15007;		//	15007 	 Limit Switch High/Low Status		

};

struct NMX_Debug_Commands {
			
	/********************************

		9 -- Debug commands

			0 -- All commands

	**********************************/

	/** Commands **/ 

		const unsigned int

		LED			= 08000,		//	09000 	 Debug LED
		COM_OUT		= 09001,		//	09001 	 COM_OUT
		STEPS		= 09002,		//	09002 	 STEPS
		MOTOR		= 09003,		//	09003 	 MOTOR
		GEN_SER		= 09004,		//	09004 	 GEN_SER
		FUNCT		= 09005,		//	09005 	 FUNCT
		CONFIRM		= 09006;		//	09006 	 CONFIRM

};

#endif

