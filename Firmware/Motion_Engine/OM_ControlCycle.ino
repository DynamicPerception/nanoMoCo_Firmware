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
  Main Timing Cycle functions - initialize StateEngine (OMState)
  ========================================
  
*/


unsigned long	camera_tm	= 0;
byte			altBlock	= 0;


void setupControlCycle() {

 Engine.setHandler(ST_CLEAR, cycleCamera);
 Engine.setHandler(ST_MOVE, cycleClearToMove);
 Engine.setHandler(ST_RUN, cycleCheckMotor);
 Engine.setHandler(ST_EXP, camExpose);
 Engine.setHandler(ST_WAIT, camWait);
 Engine.setHandler(ST_ALTP, cycleCheckAltPost);
}



void cycleCamera() {
	const String CYCLE_CAMERA = "cycleCamera() - ";
	debug.functln(CYCLE_CAMERA + "Start");

	// Check to see if a pause was requested. The program is paused here to avoid unexpected stops in the middle of a move or exposure.
	if (pause_flag) {		
		debug.functln(CYCLE_CAMERA + "Pausing");
		pauseProgram();
	}


	// Determine whether the critera for denoting a completed SMS or continuous program have been met
	bool sms_done = false;
	bool continuous_done = false;
	if (Motors::planType() == SMS)
		sms_done = Camera.getMaxShots() > 0 && camera_fired > Camera.getMaxShots();		
	else if (Motors::planType() != SMS){
		continuous_done = true;
		for (byte i = 0; i < MOTOR_COUNT; i++){
			if (!motor[i].programDone())
				continuous_done = false;
		}
	}

	/*
	* Stop program if max shots exceeded or if the continuous TL/video program as reached its destination.
	* The program stops when camera_fired exceeds Camera.maxShots instead of equalling it in order to allow the camera to take an exposure at its final destination.
	* If the keep_camera_alive variable is true, then the program will not stop (i.e. the camera will continue shooting) until the program is manually stopped by
	* pressing the e-stop button.
	*/
	if (!keep_camera_alive && (sms_done || continuous_done)) {

	  bool ready_to_stop = true;

	  // If this is a video move there might be a lead-out to wait for, so if the run time is less than the total calculated time, don't stop the program yet
	  // totalProgramTime() and last_run_time are unsigned, so they must be recast as signed values to avoid rolling the result if last_runt_time is longer
	  if (Motors::planType() != SMS && ((long)totalProgramTime() - (long)last_run_time) > 0) {
		  
		  ready_to_stop = false;

		  // Debug output
		  debug.functln(CYCLE_CAMERA + "All mot done moving");
		  if ((totalProgramTime() - last_run_time) > 0) {
			  debug.funct(totalProgramTime());
			  debug.funct(last_run_time);
			  debug.funct(CYCLE_CAMERA + "There are ");
			  debug.funct((long)totalProgramTime() - (long)last_run_time);
			  debug.functln("ms of lead-out remaining");
		  }	
	  }

		// stop program running w/o clearing variables
		if (ready_to_stop) {
			stopProgram();
			program_complete = true;

			// If not running a ping-pong move, activate the camera trigger to stop the video recording
			if (!ping_pong_mode && Motors::planType() == CONT_VID)
				Camera.expose();

			// If ping pong mode is active and this is a continuous video shot, reverse direction and start the program again
			else if (ping_pong_mode && Motors::planType() == CONT_VID) {
				ping_pong_flag = true;
				reverseStartStop();
				startProgram();
			}

		}
		debug.functln(CYCLE_CAMERA + "Bailing from camera cycle at point 1");
		return;
	}
	// If either the SMS or continuous move is complete and the camera is in "keep alive" mode
	// indicate that the camera is still shooting
	else if (keep_camera_alive && (sms_done || continuous_done)) {
		still_shooting_flag = true;
	}
  
  // if in external interval mode, don't do anything if a force shot isn't registered
  if (altExtInt && !altForceShot) {
	  debug.functln(CYCLE_CAMERA + "Skipping shot, waiting for external trigger");
	  return;
  }
		
	// trigger any outputs that need to go before the exposure
	if( (ALT_OUT_BEFORE == altInputs[0] || ALT_OUT_BEFORE == altInputs[1]) && cycleShotOK(true) ) {
		altBlock = ALT_OUT_BEFORE;
		altOutStart(ALT_OUT_BEFORE);		
		debug.functln(CYCLE_CAMERA + "Bailing from camera cycle at point 2");
		return;
	}
	
	
    // if enough time has passed, and we're ok to take an exposure
    // note: for slaves, we only get here by a master signal, so we don't check interval timing

  if( ComMgr.master() == false || ( millis() - camera_tm ) >= Camera.intervalTime() || !Camera.enable || external_intervalometer ) {

	  debug.functln(CYCLE_CAMERA + "Shots: ");
	  debug.funct(camera_fired);
	  debug.funct(" ");
	  for (byte i = 0; i < MOTOR_COUNT; i++){
		  debug.funct("Motor ");
		  debug.funct(i);
		  debug.funct(": ");
		  debug.funct(motor[i].currentPos());
		  debug.funct(" ");
	  }
	  debug.functln("");	  

      // skip camera actions if camera disabled  
      if( ! Camera.enable ) {
        Engine.state(ST_MOVE);
        camera_tm = millis();  
		
		debug.functln(CYCLE_CAMERA + "Bailing from camera cycle at point 3");
        return;
      }
	  
	  // trigger focus, if needed, which will set off the chain of
      // callback executions that will walk us through the complete exposure cycle.
      // -- if no focus is configured, nothing will happen but trigger
      // the callback that will trigger exposing the camera immediately
	  debug.functln(CYCLE_CAMERA + "Camera busy: ");
	debug.functln(Camera.busy());
	
    if( ! Camera.busy() ) {
		debug.functln(CYCLE_CAMERA + "Starting exposure cycle");
		// only execute cycle if the camera is not currently busy
		Engine.state(ST_BLOCK);
		altBlock = ALT_OFF;
		altForceShot = false;
		camera_tm = millis();  
		Camera.focus();
    } 
    
  }
  
}

/** OK To Start a Shot Sequence?

 Checks whether or not a shot sequence (or pre-shot alt out trigger) is ok to execute.
 
 @param p_prealt
 Whether to check for a pre-shot alt out trigger (true) or normal shot sequence (false)
 
 @return
 True if good to go, false otherwise
 
 @author
 C. A. Church
 */
 
uint8_t cycleShotOK(uint8_t p_prealt) {
	
	const String CYCLE_SHOT_OK = "cycleShotOK() - ";

	debug.functln(CYCLE_SHOT_OK + "Enter function");

    // if we're in alt i/o as external intervalometer mode...
	  if( altExtInt ) {
		  debug.functln(CYCLE_SHOT_OK + "Ext. interval mode active");
			// don't do a pre-output clearance if alt_block is true...
		  if( p_prealt && altBlock )
			return false;
        
			// determine whether or not to fire based on alt_force_shot
		  if (altForceShot == true) {			  
			  debug.functln(CYCLE_SHOT_OK + "altForceShot detected ************");
			  return true;
		  }
		  else {
			  debug.functln(CYCLE_SHOT_OK + "altForceShot not detected");
			  return false;
		  }
	  }
  
	// pre--output clearance check
	if(altBeforeDelay >= Camera.intervalTime() && !altBlock){  //Camera.intervalTime() is less than the altBeforeDelay, go as fast as possible
		return true;
	} 
	else if( (millis() - camera_tm) >= (Camera.intervalTime() - altBeforeDelay)  && ! altBlock )
		return true;

  
	return false;
  
}


/** Move Motors Callback Handler

  Executes any required move
  
  @author
  C. A. Church
  */

void cycleClearToMove() {
	
       // signal any slaves that they're ok to proceed, if master
       ComMgr.masterSignal();
       
	   int minPlanLead = 0;
	   
       // do not move if a motor delay is programmed...
	   for(int i = 0; i < MOTOR_COUNT; i++){
		   
		   if( (motor[i].enable())){
			   if ((minPlanLead != 0 && motor[i].planLeadIn() < minPlanLead) || (i == 0))
					minPlanLead = motor[i].planLeadIn();
		   }  

	   }
	   
	   //do not move until the minimum plan lead in has passed, if planType() == CONT_VID then the plan lead in is in ms
	   if ((minPlanLead > 0 && ((camera_fired <= minPlanLead && Motors::planType() != CONT_VID) || (Motors::planType() == CONT_VID && run_time <= minPlanLead)))) {
		   	Engine.state(ST_CLEAR);
		   	return;
	   	}
      
         // ok to run motors, if needed
      move_motor(); 
}


/** Check Motor Status Callback Handler

 This callback handler handles the ST_RUN state.  
 
 If continuous motion is requested, sets back to clear to fire state, no matter if motors
 are running.
 
 If interleaved (SMS) motion is requested, blocks fire state until all movement is complete.
 
 @author
 C. A. Church
 */


void cycleCheckMotor() {
         // still running

     // do not block on continuous motion of any sort
	if (OMMotorFunctions::planType() == SMS){
		for (int i = 0; i < MOTOR_COUNT; i++){
			if (motor[i].running() == true)
				return;
		}
	}

    // no longer running, ok to fire camera

    
    if( ComMgr.master() == true ) {
        // we are a timing master
      Engine.state(ST_CLEAR);    
        
        // if autopause is enabled then pause upon completion of movement
      if( motor[0].autoPause == true || motor[1].autoPause == true || motor[2].autoPause == true ) {
		  debug.functln("Auto pausing!!!");
          pauseProgram();
      }
    }
    else { 
        // we are a slave - block until next slaveClear signal
      Engine.state(ST_BLOCK);
     }
}


/** Check Alt Output Post Trigger

 This callback handler handle the ST_ALTP state.
 
 If one or more I/Os are configured as post-exposure outputs, this state
 will cause a (non-blocking) run delay for the specified output delay time, and then
 trigger the outputs to fire.
 
 @author
 C. A. Church
 */

void cycleCheckAltPost() {
  
  static unsigned long alt_tm = millis();
  
    // no output after set, move on to move...
  if( ! (ALT_OUT_AFTER == altInputs[0] || ALT_OUT_AFTER == altInputs[1]))
  {
    Engine.state(ST_MOVE);
  } else if(  ! altBlock ) {		//output after is set but hasn't been initiated yet 
    altBlock = ALT_OUT_AFTER;
    alt_tm = millis();
  }
  else if( ( millis() - alt_tm ) > altAfterDelay ) {	//output after is set and enough time has pass to initiate it
    altBlock = ALT_OFF;
    altOutStart(ALT_OUT_AFTER);
  }
  
}




