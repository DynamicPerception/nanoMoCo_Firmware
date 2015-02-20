/****************************

	Belt Cutter Variables

*****************************/


// Enum the constants used for the belt cutter commands
enum cutter_constants { FORWARD_6IN, FORWARD_12IN, BACK_6IN, BACK_12IN, CUTTER, AUTO_4, AUTO_6, AUTO_12, AUTO_15 };

const byte CUTTER_MOTOR = 0; // The belt cutter only has one motor, so always use motor 0
byte cutter_repeats = 0;


/****************************

	Belt Cutter Functions

*****************************/


/*

void cutterRepeats(byte p_count)

Set the number of times to repeat the requested automated belt cutter function.

*/

void cutterRepeats(byte p_count) {
	cutter_repeats = p_count;
}


/*

byte cutterRepeats() {

Gets the number of times to repeat the requested automated belt cutter function.

*/

byte cutterRepeats() {
	return cutter_repeats;
}


/*

void cutterSerial(byte p_cutter_command)

Handles incoming serial requests for belt cutter commands.

p_cutter_command: 

	0 - Forward 6 inches
	1 - Forward 12 inches
	2 - Backward 6 inches
	3 - Backward 12 inches
	4 - Manual cut
	5 - Auto cut 4 feet
	6 - Auto cut 6 feet
	7 - Auto cut 12 feet
	8 - Auto cut 15 feet

The possible values for this parameter are populated in enum cutter_constants.

*/

void cutterSerial(byte p_cutter_command) {
	const byte FORWARD = 1;
	const byte BACK = 0;

	// Set the bet cutter motor to full stepping for fastest operation.  Also, the stepCalculator calculates in full steps,
	// so it needs to be in full stepping to produce an accurate belth length.
	motor[CUTTER_MOTOR].ms(1);
	motor[CUTTER_MOTOR].contSpeed(4000); // Set the motor to a high, but reasonable speed

	switch (p_cutter_command) {
		case FORWARD_6IN:
			runCutter(0, 6, FORWARD);
			break;
		case FORWARD_12IN:
			runCutter(0, 12, FORWARD);
			break;
		case BACK_6IN:
			runCutter(0, 6, BACK);
			break;
		case BACK_12IN:
			runCutter(0, 12, BACK);
			break;
		case CUTTER:
			cutBelt();
			break;
		case AUTO_4:
			runCutter(4, 0, FORWARD, true, cutterRepeats());
			break;
		case AUTO_6:
			runCutter(6, 0, FORWARD, true, cutterRepeats());
			break;
		case AUTO_12:
			runCutter(12, 0, FORWARD, true, cutterRepeats());
			break;
		case AUTO_15:
			runCutter(15, 0, FORWARD, true, cutterRepeats());
			break;
	}
}


/*

void runCutter(byte p_feet, byte p_inches, byte dir, bool p_cut = false, byte p_repeats = 0)

Executes belt cutter moves. 

p_feet:					Number of feet to feed
p_inches:				Number of inches to feed
p_dir:					Motor direction
p_cut (optional):		Whether to actuate the cutter after feeding (default = false)	
p_repeats (optional):	Number of times the action should be repeated (default = 0)

*/

void runCutter(byte p_feet, byte p_inches, byte dir, bool p_cut = false, byte p_repeats = 0) {
	
	// Execute the move the requested number of times
	for (byte i = 0; i < p_repeats; i++) {
		simpleMove(CUTTER_MOTOR, dir, stepCalculator(p_feet, p_inches));
		while (motor[CUTTER_MOTOR].running()) {
			// Wait until motor is done running to proceed
		}
		if (p_cut)
			cutBelt();
	}
	
}

/*

unsigned long stepCalculator(int p_feet, int p_inches)

Returns the number of *full* steps needed to move the requested distance. This
is primarily meant for the DP belt cutter, but could be enchanced later for general
purpose, fixed distance moves.

*/

unsigned long stepCalculator(int p_feet, int p_inches) {

	// If either paremeter is negative, we have a problem. Return 0 steps to avoid problems
	if (p_feet < 0 || p_inches < 0)
		return 0;

	const float TOOTH_PITCH = 5;				// 5mm per tooth on standard DP pulley
	const float TOOTH_COUNT = 17;				// 17 teeth on standard DP pulley
	const float MM_PER_INCH = 25.4;
	const float GEAR_RATIO = 19 + (38 / 187);	// Gear ratio of Ningbo-Leison gearheads
	const float STEPS_PER_ROT = 200;			// Steppers have 200 steps per rotation
	const byte INCHES_PER_FOOT = 12;


	float steps_per_inch = (MM_PER_INCH * GEAR_RATIO * STEPS_PER_ROT) / (TOOTH_PITCH * TOOTH_COUNT);
	int inches = (p_feet * INCHES_PER_FOOT) + p_inches;
	unsigned long steps = (unsigned long)round(steps_per_inch * (float)inches);

	return steps;
}

/*

void cutBelt()

After waiting for all motors to stop moving, brings the aux tip high. On the DP belt cutter,
the aux tip should be wired to a relay that in turn will trigger the cutter solenoid.

*/

void cutBelt() {

	unsigned long time = millis();
	const unsigned int TIME_OUT = 15000; // Time-out delay in milliseconds

	// Make sure the aux pin is set as an output
	pinMode(AUX_TIP, OUTPUT);

	while (motor[0].running() || motor[1].running() || motor[2].running()) {
		// Wait for the motors to stop moving, but if it takes longer than the time-out, bail on the command		
		if (millis() - time > TIME_OUT)
			return;
	}

	digitalWrite(AUX_TIP, HIGH);
	// Wait half a second for the cutter to complete its operation
	delay(500);
	digitalWrite(AUX_TIP, LOW);
}