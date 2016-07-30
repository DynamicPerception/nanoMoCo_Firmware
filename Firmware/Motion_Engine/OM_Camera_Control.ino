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

  ========================================
  Camera control functions
  ========================================
  
*/

 

uint8_t oldProgramMode;

void camExpose() {
  
   // what to do when camera focus is complete?
   
    // state to block must happen before call to expose()
  Engine.state(ST_BLOCK); // block further activity until exposure is done
  Camera.expose();
}

void camWait() {
  
   // what to do when camera exposure is complete?
   
   
    // state to block must happen before call to wait()
  Engine.state(ST_BLOCK); // block further activity until post delay is done
  Camera.wait();
}


void camCallBack(byte code) {
    
  // This callback is called whenever an asynchronous activity begins or ends, and 
  // is able to take some actions as different states are reported out of the
  // camera object.
  //
  // We only care about when certain activities complete, so that's what we look for..
  //
  // Do NOT attempt to use delay() here, or call another camera action directly, as this
  // function is called in an interrupt and can daisy-chain under certain configurations,
  // which can result in unexpected behavior
  
  if( code == OM_CAM_FFIN ) {
	  debug.functln(F("camCallBack() - Start"));
	  Engine.state(ST_EXP);
  }
  else if( code == OM_CAM_EFIN ) {	
	debug.functln(F("camCallBack() - Stop"));
	camera_fired++;
    Engine.state(ST_WAIT);
  }
  else if( code == OM_CAM_WFIN ) {
      // we may have exposure repeat cycle to manage
      // after the post-exposure delay
    checkCameraRepeat();
  }
  

}

 // check for camera repeat cycle
void checkCameraRepeat() {
	
  
    static byte repdone = 0;
    
      // if we don't have camera repeat function enabled,
      // then go ahead and clear for alt out post shot check
    if( Camera.repeat == 0 ) {
      // No idea while state 6 will never execute but this is a fix for it
      if ( ! (ALT_OUT_AFTER == altInputs[0] || ALT_OUT_AFTER == altInputs[1]))
        Engine.state(ST_MOVE);
      else
        Engine.state(ST_ALTP);
      return;
    }
    
   if( repdone >= Camera.repeat ) {
       // we've done all of the repeat cycles
     repdone = 0;
      // No idea while state 6 will never execute but this is a fix for it
      if ( ! (ALT_OUT_AFTER == altInputs[0] || ALT_OUT_AFTER == altInputs[1]))
        Engine.state(ST_MOVE);
      else
        Engine.state(ST_ALTP);
     return;
   }
   
     // trigger another exposure
   repdone++;
   Engine.state(ST_EXP);
}

/**
Start or stop camera test mode
*/
void cameraTest(uint8_t p_start) {

	// If the command doesn't change the test mode, ignore it and exit the fucntion
	if (camera_test_mode == p_start)
		return;

	Camera.enable = true;
	static uint8_t old_enable[MOTOR_COUNT];
	static unsigned long old_max_shots;
	camera_test_mode = p_start;

	// Entering test mode
	if (camera_test_mode) {
		
		// Save current program type, then switch to SMS mode
		oldProgramMode = Motors::planType();
		Motors::planType(SMS);

		// Remember each motor's enable mode and disable all of them
		for (byte i = 0; i < MOTOR_COUNT; i++) {
			old_enable[i] = motor[i].enable();
			motor[i].enable(false);
		}

		// Remember the current max shots setting
		old_max_shots = Camera.getMaxShots();

		// Set the max shots to an arbitrarily large value so the test mode doesn't stop
		Camera.setMaxShots(10000);

		// Starting the program will make the camera fire, but the motors won't move
		startProgram();

	}

	// Exiting test mode
	else if (!camera_test_mode) {

		// Stop the camera firing
		stopProgram();

		// Restore motor enable statuses and camera max shots
		for (byte i = 0; i < MOTOR_COUNT; i++)
			motor[i].enable(old_enable[i]);

		Camera.setMaxShots(old_max_shots);

		// Reset the shot count to 0
		camera_fired = 0;

		// Restore old program type
		Motors::planType(oldProgramMode);
	}
}


/**

Return whether the camera is in test mode

*/
uint8_t cameraTest() {

	return(camera_test_mode);

}


/**

Automatically sets the max shots value based upon the lead-in, travel, and lead-out shots.

*/
void cameraAutoMaxShots() {

	// This function should only be used with SMS mode, since leads and travel are in milliseconds for CONT_TL and CONT_VID
	if (Motors::planType() != SMS)
		return;

	unsigned int longest = 0;
	unsigned int current = 0;

	// Find the longest combination of leads and travel, then set that as the max shots
	for (byte i = 0; i < MOTOR_COUNT; i++) {
		current = motor[i].planLeadIn() + motor[i].planTravelLength() + motor[i].planLeadOut();
		if (current > longest)
			longest = current;
	}

	Camera.setMaxShots(longest);
}

// Returns the total number of shots from the current program pass, plus any previous passes
unsigned int getTotalShots(){
	if (!running && !kf_running)
		return 0;
	else
		return camera_fired + ping_pong_shots;
}

// Clears shot counters
void clearShotCounter(){
	camera_fired = 0;
	ping_pong_shots = 0;
}





