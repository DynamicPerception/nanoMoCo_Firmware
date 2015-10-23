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

boolean kf_okForSmsMove;
int kf_curSmsFrame;

void printKeyFrameData(){

	for (byte i = 0; i < KeyFrames::getAxisCount(); i++){
		// Indicate the current axis
		USBSerial.print("~~~~~~ AXIS ");
		USBSerial.print(i);
		USBSerial.println(" ~~~~~~");
		USBSerial.println("");

		// Print abscissas
		USBSerial.println("*** Abscissas ***");
		for (byte j = 0; j < kf[i].getKFCount(); j++){
			USBSerial.println(kf[i].getXN(j));
		}
		USBSerial.println("");

		// Print Positions
		USBSerial.println("*** Positions ***");
		for (byte j = 0; j < kf[i].getKFCount(); j++){
			USBSerial.println(kf[i].getFN(j));
		}
		USBSerial.println("");

		// Print velocities
		USBSerial.println("*** Velocities ***");
		for (byte j = 0; j < kf[i].getKFCount(); j++){
			USBSerial.println(kf[i].getDN(j));
		}
		USBSerial.println("");
		USBSerial.println("");
	}

}

void startKFProgram(){
	
	// If resuming
	if (kf_program_paused){

		USBSerial.println("Resuming KF program");

		// Add the time of the last pause to the total pause time counter
		kf_pause_time += kf_this_pause;
	}
	// If starting a new program
	else{

		USBSerial.println("Starting new KF program");

		// Reset the SMS vars
		kf_okForSmsMove = false;
		kf_curSmsFrame = 0;

		// Reset the total pause time counter
		kf_pause_time = 0;

		// Reset the camera shot time
		kf_last_shot_tm = 0;

		// Make sure the pause flag is off
		kf_program_paused = false;

		// Turn on joystick mode
		joystickSet(true);

		// Determine the max running time
		USBSerial.println("Finding max time...");
		kf_max_time = 0;
		for (byte i = 0; i < KeyFrames::getAxisCount(); i++){
			int lastFrame = kf[i].getKFCount() - 1;
			USBSerial.print("Frame count: ");
			USBSerial.println(lastFrame + 1);
			long this_time = kf[i].getXN(lastFrame);
			USBSerial.print("this_time: ");
			USBSerial.println(this_time);

			if (this_time > kf_max_time)
				kf_max_time = this_time;
		}

		if (Motors::planType() == SMS){
			// Convert from "frames" to real milliseconds, based upon the camera interval
			kf_max_time = ((float)Camera.maxShots / MILLIS_PER_FRAME) * Camera.intervalTime();
			USBSerial.print("Camera interval");
			USBSerial.println(Camera.intervalTime());
			USBSerial.print("Program run time in milliseconds: ");
			USBSerial.println(kf_max_time);
		}

		// Take up any motor backlash
		USBSerial.println("Taking up backlash...");
		takeUpBacklash();

		// Initialize the run timers
		USBSerial.println("Initializing run timers...");
		kf_run_time = 0;
		kf_start_time = millis();
		kf_last_update = millis();

		// Set the initial motor speeds
		USBSerial.println("Setting initial motor speeds...");
		for (byte i = 0; i < MOTOR_COUNT; i++){
			// Don't touch motors that don't have any key frames
			if (kf[i].getKFCount() > 0)
				setJoystickSpeed(i, kf[i].vel(0) * MILLIS_PER_SECOND);
		}
	}

	// Turn on the key frame program flag and turn the paused flag off
	kf_program_running = true;
	kf_program_paused = false;
}

void pauseKFProgram(){

	if (usb_debug & DB_FUNCT)
		USBSerial.print("PAUSING KF PROGRAM");

	// Stop all motors
	for (byte i = 0; i < MOTOR_COUNT; i++){
		USBSerial.print("Stopping motor ");
		USBSerial.println(i);
		setJoystickSpeed(i, 0);
	}

	// Set the pause flag
	kf_program_paused = true;

	// Reset the current pause duration counter
	kf_this_pause = 0;

	// Log the start time of the pause
	kf_pause_start = millis();	
}

void stopKFProgram(){

	if (usb_debug & DB_FUNCT)
		USBSerial.print("STOPPING KF PROGRAM");

	// Make sure all motors are stopped
	for (byte i = 0; i < MOTOR_COUNT; i++){
		setJoystickSpeed(i, 0);
	}

	// Disable joystick mode
	joystickSet(false);

	// Turn off the key frame program flag
	kf_program_running = false;
	kf_program_paused = false;
}

void updateKFProgram(){

	// If the program is paused, just keep track of the pause time
	if (kf_program_paused){		
		kf_this_pause = millis() - kf_pause_start;
		if (usb_debug & DB_FUNCT){
			USBSerial.print("Pause length: ");
			USBSerial.println(kf_this_pause);
		}
		return;
	}

	// Update run_time, don't include time spent paused
	kf_run_time = millis() - kf_start_time - kf_pause_time;

	if (usb_debug & DB_FUNCT){
		USBSerial.print("Run time: ");
		USBSerial.println(kf_run_time);
	}

	
	// Continuous move update	
	if (Motors::planType() != SMS){
		updateKfContSpeed();
	}

	// Check whether the camera needs to fire
	kfCameraCheck();

	// SMS move update
	if (Motors::planType() == SMS){
		updateKfSMS();
	}

	// Check to see if the program is done
	if (kf_run_time > kf_max_time){
		stopKFProgram();
	}
}

void updateKfContSpeed(){	

	// If the update time has elapsed, update the motor speed
	if (millis() - kf_last_update > KeyFrames::updateRate()){

		for (byte i = 0; i < MOTOR_COUNT; i++){

			// Determine the maximum run time for this axis
			float thisAxisMaxTime = kf[i].getXN(kf[i].getKFCount() - 1);
			if (Motors::planType() == SMS)
				thisAxisMaxTime = thisAxisMaxTime / MILLIS_PER_FRAME * Camera.intervalTime();

			// Set the approriate speed, but don't touch motors that don't have any key frames
			if (kf[i].getKFCount() > 0){
				float speed;
				if (kf_run_time > thisAxisMaxTime)
					speed = 0;
				else
					speed = kf[i].vel((float)kf_run_time) * MILLIS_PER_SECOND; // Convert from steps/millisecond to steps/sec
				setJoystickSpeed(i, speed);
			}
		}
		kf_last_update = millis();
	}
}

void updateKfSMS(){	
	
	// If we're not ready for a move (i.e. the camera is busy or we just finished one), don't do anything
	if (!kf_okForSmsMove){
		USBSerial.println("Updating SMS: not ready for move");
		return;
	}

	// Send the motors to their next locations
	USBSerial.println("Sending motors to new locations");
	for (int i = 0; i < MOTOR_COUNT; i++){		
		float nextPos = kf[i].pos(kf_curSmsFrame+1);
		motor[i].moveTo(nextPos);
	}	
	kf_curSmsFrame++;
	kf_okForSmsMove = false;
}

void kfCameraCheck() {

	static boolean auxFired = false;
	static boolean auxDone = false;
	static boolean focusFired = false;
	static boolean focusDone = false;
	static boolean shutterFired = false;
	static boolean shutterDone = false;
	static boolean forceShotInProgress = false;
	int auxPreShotTime = 0;

	// If in external interval mode, don't do anything if a force shot isn't registered
	if (altExtInt && !altForceShot && !forceShotInProgress) {
		if (usb_debug & DB_FUNCT)
			USBSerial.println("cycleCamera() - Skipping shot, waiting for external trigger");
		return;
	}

	// If there's an aux event before the camera events, set the time needed for that
	if (ALT_OUT_BEFORE == altInputs[0] || ALT_OUT_BEFORE == altInputs[1]){
		auxPreShotTime = altBeforeDelay;
	}

	boolean auxLongerThanInt = auxPreShotTime >= Camera.intervalTime();
	boolean timeForNewShot = (kf_run_time - kf_last_shot_tm) >= (Camera.intervalTime() - auxPreShotTime);

	// If it's time, start a new shot
	if (((auxLongerThanInt || timeForNewShot) && !altBlock) || altForceShot){  //Camera.intervalTime() is less than the altBeforeDelay, go as fast as possible
		
		kf_last_shot_tm = kf_run_time;
		
		auxFired = false;
		auxDone = false;
		focusFired = false;
		focusDone = false;
		shutterFired = false;
		shutterDone = false;	
		
		if (altForceShot){
			forceShotInProgress = true;
			altForceShot = false;
		}
	}	

	// Trigger any outputs that need to go before the exposure
	if ((ALT_OUT_BEFORE == altInputs[0] || ALT_OUT_BEFORE == altInputs[1]) && cycleShotOK(true) && !auxDone) {
		if (!auxFired){
			altBlock = ALT_OUT_BEFORE;
			altOutStart(ALT_OUT_BEFORE);
			if (usb_debug & DB_FUNCT)
				USBSerial.println("cycleCamera() - Bailing from camera cycle at point 2");
			auxFired = true;
			return;
		}
		
		// If not enough time for the aux function has elapsed, return
		if (kf_run_time < kf_last_shot_tm + auxPreShotTime){
			return;
		}
		auxDone = true;
	}

	altBlock = ALT_OFF;
	altForceShot = false;
		
	// Trigger the focus
	if (!focusDone){
		if (!focusFired){
			USBSerial.print("Time at focus: ");
			USBSerial.println(kf_run_time);
			Camera.focus();
			focusFired = true;
			return;
		}

		// If not enough time for the focus has elapsed, return
		if (kf_run_time < kf_last_shot_tm + auxPreShotTime + Camera.focusTime()){
			USBSerial.println("Waiting for focus to finish");
			return;
		}
		focusDone = true;
	}
	
	// Trigger the exposure
	if (!shutterDone){
		if (!shutterFired){
			USBSerial.print("Time at exposure: ");
			USBSerial.println(kf_run_time);		
			Camera.expose();
			shutterFired = true;
		}

		// If not enough time for the exposure has elapsed, return
		if (kf_run_time < kf_last_shot_tm + auxPreShotTime + Camera.focusTime() + Camera.triggerTime() + Camera.delayTime()){
			USBSerial.println("Waiting for shutter to finish");
			return;
		}		
		shutterDone = true;
		forceShotInProgress = false;

		// One the camera functions are complete, it's okay to make the next SMS move
		USBSerial.println("Shutter done, ready for move");
		kf_okForSmsMove = true;
	}

}

void kf_auxCycleStart(byte p_mode) {

	//Pins for the I/O
	const byte           AUX_RING = 24;				//PD0
	const byte            AUX_TIP = 25;				//PD1

	uint8_t altStarted = false;

	unsigned int adelay = p_mode == ALT_OUT_BEFORE ? altBeforeMs : altAfterMs;

	for (byte i = 0; i < 2; i++) {
		if (p_mode == altInputs[i]) {

			// note that alt 3 is on a different register..
			if (i == 0)
				digitalWrite(AUX_RING, altOutTrig);
			else
				digitalWrite(AUX_TIP, altOutTrig);

			altStarted = true;
		}
	}

	if (altStarted) {
		MsTimer2::set(adelay, altOutStop);
		MsTimer2::start();
		Engine.state(ST_BLOCK);
	}
	else if (p_mode == ALT_OUT_BEFORE) {
		//clear to shoot
		Engine.state(ST_CLEAR);
	}
	else {
		// clear to move
		Engine.state(ST_MOVE);
	}

}

/*
	This function compares the max SMS speeds for each
	axis against the maximum possible speed for each axis.
	If at least one of them exceeds the axis' max speed,
	the function returns false.
*/
boolean kfValidateSMSProgram(){
	for (byte i = 0; i < MOTOR_COUNT; i++){
		float thisMaxSpeed = kfMaxSMSSpeed(i);

		// Return false if one of the segments requires a speed higher than is possible
		if (thisMaxSpeed > motor[i].maxSpeed()){
			return false;
		}
	}
	// If everything checks out, return true
	return true;
}

/*
	This function finds the maximum steps per second required 
	for the requested axis' SMS key frame program. Only use if  
	an SMS key frame program has already been set!
*/
float kfMaxSMSSpeed(int axis){
		
	float maxTime = kf[axis].getXN(kf[axis].getKFCount() - 1);
	int kfCount = kf[axis].getKFCount();
	float timeInc = maxTime / kfCount;
	float maxSpeed = 0;

	// For each key frame for that axis
	for (byte i = 0; i < kfCount; i++){

		// Find step difference for this segment
		float startTime = timeInc * i;
		float stopTime = timeInc * (i + 1);
		float startStep = kf[axis].pos(startTime);
		float stopStep = kf[axis].pos(stopTime);

		float stepsPerSec = (stopStep - startStep) / timeInc;

		if (stepsPerSec > maxSpeed)

		// Return false if one of the segments requires a speed higher than is possible
		if (stepsPerSec > maxSpeed){
			maxSpeed = stepsPerSec;
		}
	}
	return maxSpeed;
}