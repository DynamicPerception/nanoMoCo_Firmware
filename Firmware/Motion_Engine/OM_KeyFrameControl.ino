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

	USBSerial.print("Starting KF program in mode: ");
	USBSerial.println(motor[0].planType());

	// Turn on the key frame program flag
	kf_program_running = true;

	// Turn on joystick mode
	joystickSet(true);

	// Determine the max running time
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

	if (motor[0].planType() == SMS){
		// Convert from "frames" to real milliseconds, based upon the camera interval
		kf_max_time = ((float)max_time / MILLIS_PER_FRAME) * Camera.interval;
	}

	USBSerial.print("Max run time: ");
	USBSerial.println(kf_max_time);


	// Initialize the run timers
	kf_run_time = 0;
	kf_start_time = millis();
	kf_last_update = millis();

	// Set the initial motor speeds
	for (byte i = 0; i < MOTOR_COUNT; i++){
		// Don't touch motors that don't have any key frames
		if (kf[i].getKFCount() > 0)
			setJoystickSpeed(i, kf[i].vel(0) * MILLIS_PER_SECOND);
	}
}

void updateKFProgram(){

	// Update run_time
	kf_run_time = millis() - kf_start_time;

	// If the update time has elapsed, update the motor speed
	if (millis() - kf_last_update > KeyFrames::updateRate()){
		for (byte i = 0; i < MOTOR_COUNT; i++){

			// Determine the maximum run time for this axis
			float thisAxisMaxTime = kf[i].getXN(kf[i].getKFCount() - 1);
			if (motor[0].planType() == SMS)
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
