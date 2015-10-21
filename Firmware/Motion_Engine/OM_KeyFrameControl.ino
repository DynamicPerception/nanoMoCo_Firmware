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

		// Reset the total pause time counter
		kf_pause_time = 0;

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
			kf_max_time = ((float)max_time / MILLIS_PER_FRAME) * Camera.interval();
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
		//USBSerial.print("Pause length: ");
		//USBSerial.println(kf_this_pause);
	}

	if (Motors::planType() == SMS){

	}
	// Update run_time, don't include time spent paused
	else{
		
		kf_run_time = millis() - kf_start_time - kf_pause_time;
		//USBSerial.print("Run time: ");
		//USBSerial.println(kf_run_time);

		if (Motors::planType() == CONT_TL){

		}

		// If the update time has elapsed, update the motor speed
		if (millis() - kf_last_update > KeyFrames::updateRate()){
			
			for (byte i = 0; i < MOTOR_COUNT; i++){

				// Determine the maximum run time for this axis
				float thisAxisMaxTime = kf[i].getXN(kf[i].getKFCount() - 1);
				if (Motors::planType() == SMS)
					thisAxisMaxTime = thisAxisMaxTime / MILLIS_PER_FRAME * Camera.interval;

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

		// Check to see if the program is done
		if (kf_run_time > kf_max_time){

			USBSerial.println("Program complete");

			// Make sure all motors are stopped
			for (byte i = 0; i < MOTOR_COUNT; i++){
				setJoystickSpeed(i, 0);
			}
			// Disable joystick mode
			joystickSet(false);
			// Turn off the key frame program flag
			kf_program_running = false;
		}
	}
}
