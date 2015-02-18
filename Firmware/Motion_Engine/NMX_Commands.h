// NMX_Commands.h

#ifndef _NMX_COMMANDS_H
#define _NMX_COMMANDS_H

/* Command format:

0-65,535

Query		Group		Sub-group	Command #

1 or 2		0 - 9		0-9			0-99

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

For example, setting the constant speed for motor 1 might be command 11113,
while the command for querying the constant speed for motor 1 might be command 21113.

*/

struct NMX_General_Commands {

	/********************************

		0 -- General settings

			0 -- Controller settings

	**********************************/

	/** Command **/

		const char
									//  10000	Code unused
		ADDR			= 10001,	//	10001 	Set device address
		NAME			= 10002,	//	10002 	Set Stored Name
		TIMING			= 10003,	//	10003 	Timing Master
		COMMON_LINE		= 10004,	//	10004 	Set Common Line for Step Pulsing

	/**Query**/

									//	20000 	 N/A
		q_NAME			= 20001,	//	20001 	 Device name
		q_TIMING		= 20002,	//	20002 	 Timing master value
									//	20003 	 N/A
									//  20004	 N/A
		q_VERSION		= 20005,	//	20005 	 Firmware Version
		q_VOLTAGE		= 20006,	//	20006 	 Voltage Reading
		q_POWERCYCLE	= 20007;	//	20007 	 Controller Power Cycle

};


struct NMX_Motor_Commands {

	/********************************

		1 -- Motor settings

			0-2 -- Individual Motors

	**********************************/

	/** Command **/

		/** Basic settings **/

	const unsigned int

		ENABLE[MOTOR_COUNT]			= { 11000, 11100, 11200 },		//	11000 	 Motor Enable
		SLEEP[MOTOR_COUNT]			= { 11001, 11101, 11201 },		//	01001 	 Motor Sleep
		BACKLASH[MOTOR_COUNT]		= { 11002, 11102, 11202 },		//	01002 	 Set Backlash Steps
		MICROSTEPS[MOTOR_COUNT]		= { 11003, 11103, 11203 },		//	01003 	 Set Microstep Value
		MAX_SPEED[MOTOR_COUNT]		= { 11004, 11104, 11204 },		//	01004 	 Set Motor Max Step Speed

		/** Hard limits **/

		SET_HOME[MOTOR_COUNT]		= { 11010, 11110, 11210 },		//	01010 	 Set Home Limit Here
		SET_END[MOTOR_COUNT]		= { 11011, 11111, 11211 },		//	01011 	 Set End Limit Here
		SEND_HOME[MOTOR_COUNT]		= { 11012, 11112, 11212 },		//	01012 	 Send Motor to Home Limit
		SEND_END[MOTOR_COUNT]		= { 11013, 11113, 11213 },		//	01013 	 Send Motor to End Limit

		/** Simple motor control **/

		STOP[MOTOR_COUNT]			= { 11020, 11120, 11220 },		//	01020 	 Stop Motor Now
		DIR[MOTOR_COUNT]			= { 11021, 11121, 11221 },		//	01021 	 Set Direction
		CONST_SPEED[MOTOR_COUNT]	= { 11021, 11121, 11221 },		//	01022 	 Set Continuous Speed
		CONST_ACCEL[MOTOR_COUNT]	= { 11022, 11122, 11222 },		//	01023 	 Set Motor Continuous Motion Accel/Decel Rate
		MOVE_SIMPLE[MOTOR_COUNT]	= { 11023, 11123, 11223 },		//	01024 	 Execute Simple Motor Move

		/** Program limits **/

		SET_START_HERE[MOTOR_COUNT]	= { 11030, 11130, 11230 },		//	01030 	 Set Start Here
		SET_STOP_HERE[MOTOR_COUNT]	= { 11031, 11131, 11231 },		//	01031 	 Set Stop Here
		SET_START[MOTOR_COUNT]		= { 11032, 11132, 11232 },		//	01032 	 Set Program Start point
		SET_STOP[MOTOR_COUNT]		= { 11033, 11133, 11233 },		//	01033 	 Set Program Stop point
		SEND_START[MOTOR_COUNT]		= { 11034, 11134, 11234 },		//	01034 	 Send Motor to Program Start Point
		SEND_STOP[MOTOR_COUNT]		= { 11035, 11135, 11235 },		//	01035 	 Send Motor to Program Stop Point
		RESET_START_STOP[MOTOR_COUNT] = { 11036, 11136, 11236 },	//	01036 	 Reset Limits and Program Start/Stop Positions

		/** Program movement **/

		EASING[MOTOR_COUNT]			= { 11040, 11140, 11240 },		//	01040 	 Set Easing (Ramping) Mode
		LEAD_IN[MOTOR_COUNT]		= { 11041, 11141, 11241 },		//	01041 	 Set Lead-In Shots / Time
		ACCEL_PROG[MOTOR_COUNT]		= { 11042, 11142, 11242 },		//	01042 	 Set Program Accel
		TRAVEL_TIME[MOTOR_COUNT]	= { 11043, 11143, 11243 },		//	01043 	 Set Travel Shots(SMS) / Travel Time (Cont.)
		DECEL_PROG[MOTOR_COUNT]		= { 11044, 11144, 11244 },		//	01044 	 Set Program Decel
		LEAD_OUT[MOTOR_COUNT]		= { 11045, 11145, 11245 },		//	01045 	 Set lead-out shots / time
		AUTO_MS[MOTOR_COUNT]		= { 11046, 11146, 11246 },		//	01046 	 Auto Set Program Microsteps


		/** Query**/

		/** Basic settings **/

		q_ENABLE[MOTOR_COUNT]		= { 21000, 21100, 21200 },		//	11000 	 Motor Enable
		q_SLEEP[MOTOR_COUNT]		= { 21001, 21101, 21201 },		//	11001 	 Check Motor Sleep State
		q_BACKLASH[MOTOR_COUNT]		= { 21002, 21102, 21202 },		//	11002 	 Backlash Steps
		q_MICROSTEPS[MOTOR_COUNT]	= { 21003, 21103, 21203 },		//	11003 	 Microstep Value
		q_MAX_SPEED[MOTOR_COUNT]	= { 21004, 21104, 21204 },		//	11004 	 Motor Max Step Speed

		/** Hard limits **/

																	//	11010 	 N/A
		q_END[MOTOR_COUNT]			= { 21011, 21111, 21211 },		//	11011 	 End Limit Position

		/** Simple motor control **/

		q_RUNNING[MOTOR_COUNT]		= { 21020, 21120, 21220 },		//	11020 	 Motor Running
		q_DIR[MOTOR_COUNT]			= { 21021, 21121, 21221 },		//	11021 	 Direction
		q_CONST_SPEED[MOTOR_COUNT]	= { 21022, 21122, 21222 },		//	11022 	 Continuous Speed
		q_CONST_ACCEL[MOTOR_COUNT]	= { 21023, 21123, 21223 },		//	11023 	 Motor Continuous Motion Accel/Decel Rate
																	//	11024 	 N/A
		q_POS[MOTOR_COUNT]			= { 21025, 21125, 21225 },		//	11025 	 Current Motor Position

		/** Program limits **/

		q_START[MOTOR_COUNT]		= { 21030, 21130, 21230 },		//	11030 	 Program Start point
		q_STOP[MOTOR_COUNT]			= { 21031, 21131, 21231 },		//	11031 	 Program Stop point
																

		/** Program movement **/

		q_EASING[MOTOR_COUNT]		= { 21040, 21140, 21240 },		//	11040 	 Easing (Ramping) Mode
		q_LEAD_IN[MOTOR_COUNT]		= { 21041, 21141, 21241 },		//	11041 	 Lead-In Shots / Time
		q_ACCEL_PROG[MOTOR_COUNT]	= { 21042, 21142, 21242 },		//	11042 	 Program Accel
		q_TRAVEL_TIME[MOTOR_COUNT]	= { 21043, 21143, 21243 },		//	11043 	 Travel Shots(SMS) / Travel Time (Cont.)
		q_DECEL_PROG[MOTOR_COUNT]	= { 21044, 21144, 21244 },		//	11044 	 Program Decel
		q_LEAD_OUT[MOTOR_COUNT]		= { 21045, 21145, 21245 },		//	11045 	 Lead-Out Shots / Time
		q_VALIDATE[MOTOR_COUNT]		= { 21046, 21146, 21246 },		//	11046 	 Validate Motor Speed Limit


	/********************************

		1 -- Motor settings

			3 -- All Motors

	**********************************/

	/** Command **/

		SEND_ALL_HOME		= 11300,	//	01300 	 Return Home All Motors
		MAX_STEP_RATE		= 11301,	//	01301 	 Motors Max Step Rate
		JOYSTICK			= 11302,	//	01302 	 Set Joystick Mode
		WATCHDOG			= 11303,	//	01303 	 Set Joystick Watchdog
		SET_START_HERE		= 11304,	//	01304 	 Set Program Start point
		SET_STOP_HERE		= 11305,	//	01305 	 Set Program Stop point
		SEND_ALL_START		= 11306,	//	01306 	 Send all Motors to Program Start

	/** Query**/

										//	11300 	 N/A
		q_MAX_STEP_RATE		= 21301,	//	11301 	 Motors Max Step Rate
		q_JOYSTICK			= 21302,	//	01302 	 Joystick Mode
		q_WATCHDOG			= 21303;	//	01303 	 Joystick Watchdog Mode Status

};

struct NMX_Camera_Commands {

	/********************************

		2 -- Camera settings

			0 -- All commands

	**********************************/


	/** Control **/

		/** Simple control **/

		const unsigned int

		ENABLE				= 12000,	//	12000 	 Camera Enable
		EXPOSE				= 12001,	//	12001 	 Expose Now

		/** Program parameters **/

		INTERVAL			= 12010,	//	12010 	 Interval
		TRIGGER				= 12011,	//	12011 	 Trigger Time
		FOCUS				= 12012,	//	12012 	 Focus Time
		DELAY				= 12013,	//	12013 	 Exposure Delay
		MAX_SHOTS			= 12014,	//	12014 	 Max Shots
		FOCUS_W_SHUTTER		= 12015,	//	12015 	 Focus w Shutter
		MUP					= 12016,	//	12016 	 Mirror Up (Repeat Shot)

		/** Test Mode **/

		TEST_MODE			= 12020,	//	12020 	 Camera Test Mode

	/** Query **/

		/** Simple Control **/

		q_ENABLE			= 22000,	//	22000 	 Camera Enable
		Q_EXPOSE			= 22001,	//	22001 	 Currently Exposing

		/** Program parameters **/

		q_INTERVAL			= 22010,	//	22010 	 Interval Time
		q_TRIGGER			= 22011,	//	22011 	 Trigger Time
		q_FOCUS				= 22012,	//	22012 	 Focus Time
		q_DELAY				= 22013,	//	22013 	 Exposure Delay
		q_MAX_SHOTS			= 22014,	//	22014 	 Max Shots
		q_FOCUS_W_SHUTTER	= 22015,	//	22015 	 Focus w Shutter
		q_MUP				= 22016,	//	22016 	 Mirror Up (Repeat Shot)
		q_CURRENT_SHOTS		= 22017,	//	22017 	 Current Shots

		/** Test Mode **/

		q_TEST_MODE			= 22020;	//	22020 	 Camera Test Mode
};

struct NMX_Program_Commands {

	/********************************

		3 -- Program settings

			0 -- All commands

	**********************************/

	/** Command **/

		const unsigned int

		START				= 13000,	//	13000 	 Start
		PAUSE				= 13001,	//	13001 	 Pause
		STOP				= 13002,	//	13002 	 Stop
		MAX_RUN_TIME		= 13003,	//	13003 	 Max Program Run Time
		START_DELAY			= 13004,	//	13004 	 Start Program Delay
		MODE				= 13005,	//	13005 	 Set SMS / Continuous Program Mode
		PING_PONG			= 13006,	//	13006 	 Set Ping-Pong Flag
		FPS					= 13007,	//	13007 	 Set Frames/Second Flag


	/** Query **/

		q_STATUS			= 23000,	//	23000 	 Run Status
										//	23001 	 N/A
										//	23002 	 N/A
		q_RUN_TIME			= 23003,	//	23003 	 Elapsed Run Time
		q_START_DELAY		= 23004,	//	23004 	 Start Program Delay
		q_MODE				= 23005,	//	23005 	 SMS / Continuous Program Mode
		q_PING_PONG			= 23006,	//	23006 	 Ping-Pong Flag
		q_FPS				= 23007,	//	23007 	 Check Frames/Second Flag
		q_PERCENT_COMPLETE	= 23008,	//	23008 	 Program % Complete
		q_TOTAL_RUN_TIME	= 23009,	//	23009 	 Total Program Run Time
		q_VALIDATE			= 23010,	//	23010 	 Validate Program
		q_COMPLETE			= 23011;	//	23011 	 Program Complete?

};


struct NMX_Broadcast_Commands {

	/********************************

		4 -- Broadcast commands

			0 -- All commands

	**********************************/

	/** Commands **/
		const unsigned int

		START				= 14000,	//	14000 	 Start
		PAUSE				= 14001,	//  14001	 Pause
		STOP				= 14002,	//	14002 	 Stop
		ADDR				= 14003;	//	14003 	 Assign Address

};

struct NMX_Aux_Commands {


	/********************************

		5 -- Aux Commands

			0 -- All Commands

	**********************************/

	/** Command **/

	const unsigned int

		INPUT_EDGE			= 15000,		//	15000 	 Alt Input Edge (RISING, FALLING, or CHANGE)
		MODE				= 15001,		//	15001 	 Alt I/O Mode
		B4_SHOT_DELAY		= 15002,		//	15002 	 Alt Output Before Shot Delay Time
		AFT_SHOT_DELAY		= 15003,		//	15003 	 Alt Output After Shot Delay Time
		B4_SHOT_TIME		= 15004,		//	15004 	 Alt Output Before Shot Time
		AFT_SHOT_TIME		= 15005,		//	15005 	 Alt Output After Shot Time
		TRIG_LEVEL			= 15006, 		//	15006 	 Alt Output Trigger level


	/** Query**/

		q_INPUT_EDGE		= 25000,		//	25000 	 Alt Input Edge (RISING, FALLING, or CHANGE)
		q_MODE				= 25001,		//	25001 	 Alt I/O Mode
		q_B4_SHOT_DELAY		= 25002,		//	25002 	 Alt Output Before Shot Delay Time
		q_AFT_SHOT_DELAY	= 25003,		//	25003 	 Alt Output After Shot Delay Time
		q_B4_SHOT_TIME		= 25004,		//	25004 	 Alt Output Before Shot Time
		q_AFT_SHOT_TIME		= 25005,		//	25005 	 Alt Output After Shot Time
		q_TRIG_LEVEL		= 25006, 		//	25006 	 Alt Output Trigger level
		q_SWITCH_STATUS		= 25007;		//	25007 	 Limit Switch High/Low Status		

};

struct NMX_Debug_Commands {
			
	/********************************

		9 -- Debug commands

			0 -- All commands

	**********************************/

	/** Commands **/ 

		const unsigned int

		LED			= 18000,		//	19000 	 Debug LED
		COM_OUT		= 19001,		//	19001 	 COM_OUT
		STEPS		= 19002,		//	19002 	 STEPS
		MOTOR		= 19003,		//	19003 	 MOTOR
		GEN_SER		= 19004,		//	19004 	 GEN_SER
		FUNCT		= 19005,		//	19005 	 FUNCT
		CONFIRM		= 19006;		//	19006 	 CONFIRM

};

#endif

