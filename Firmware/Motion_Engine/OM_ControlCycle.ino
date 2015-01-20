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




unsigned long  camera_tm         = 0;
byte altBlock = 0;


void setupControlCycle() {

 Engine.setHandler(ST_CLEAR, cycleCamera);
 Engine.setHandler(ST_MOVE, cycleClearToMove);
 Engine.setHandler(ST_RUN, cycleCheckMotor);
 Engine.setHandler(ST_EXP, camExpose);
 Engine.setHandler(ST_WAIT, camWait);
 Engine.setHandler(ST_ALTP, cycleCheckAltPost);
}



void cycleCamera() {

	//for (byte i = 0; i < MOTOR_COUNT; i++){
	//	USBSerial.print("Motor ");
	//	USBSerial.print(i);
	//	USBSerial.print(" steps: ");
	//	USBSerial.print(motor[i].currentPos());
	//	USBSerial.print(" ");
	//}
	//USBSerial.println("");

	// Check to see if a pause was requested. The program is paused here to avoid unexpected stops in the middle of a move or exposure.
	if (pause_flag)
		pauseProgram();
	
  // stop program if max shots exceeded or if the continuous TL/video program as reached its destination
  // The program stops when camera_fired exceeds Camera.maxShots instead of equalling it in order to allow the camera to take an exposure at its final destination
  if((Camera.maxShots > 0  && camera_fired > Camera.maxShots) || (!Camera.enable && motor[0].programDone() && motor[1].programDone() && motor[2].programDone()) ) {
	  
	  	// stop program running w/o clearing variables
		stopProgram();
		program_complete = true;

		if (motor[0].planType() == CONT_VID)
			Camera.expose();
		
		//// If multiple key frames were set, load the parameters for the next position and start the program again
		//if (key_move && current_frame < key_frames) {
		//	for (byte i = 0; i < MOTOR_COUNT; i++) {

		//		// Re-set each motor's parameters for the next key frame
		//		motor[i].stopPos(motor[i].keyDest(current_frame));
		//		motor[i].planTravelLength(motor[i].keyTime(current_frame));
		//		motor[i].planAccelLength(motor[i].keyAccel(current_frame));
		//		motor[i].planDecelLength(motor[i].keyDecel(current_frame));
		//		motor[i].planLeadIn(motor[i].keyLead(current_frame));

		//	}
		//	current_frame++;
		//	startProgram();
		//}

		//else if (key_move && current_frame >= key_frames)
		//	current_frame = 0;
		
		// If ping pong mode is one and this is a continuous video shot, reverse direction and start the program again
		if (pingPongMode && motor[1].planType() == CONT_VID) {
			reverseStartStop();
			startProgram();
		}

		return;
  } 
  
	// if in external interval mode, don't do anything is a force shot isn't
	// registered
    
	if( altExtInt && ! altForceShot )
		return;
		
	// trigger any outputs that need to go before the exposure
	if( (ALT_OUT_BEFORE == altInputs[0] || ALT_OUT_BEFORE == altInputs[1]) && cycleShotOK(true) ) {
		altBlock = ALT_OUT_BEFORE;
		altOutStart(ALT_OUT_BEFORE);
		return;
	}
	
	
    // if enough time has passed, and we're ok to take an exposure
    // note: for slaves, we only get here by a master signal, so we don't check interval timing

  if( ComMgr.master() == false || ( millis() - camera_tm ) >= Camera.interval || !Camera.enable  ) {

	  //USBSerial.print("Shots: ");
	  //USBSerial.print(camera_fired);
	  //USBSerial.print(" ");

	  //for (byte i = 0; i < MOTOR_COUNT; i++){
		 // USBSerial.print("Motor ");
		 // USBSerial.print(i);
		 // USBSerial.print(": ");
		 // USBSerial.print(motor[i].currentPos());
		 // USBSerial.print(" ");
	  //}
	  //USBSerial.println("");

            // skip camera actions if camera disabled  
      if( ! Camera.enable ) {
        Engine.state(ST_MOVE);
        camera_tm = millis();  
        return;
      }
	  

      // trigger focus, if needed, which will set off the chain of
      // callback executions that will walk us through the complete exposure cycle.
      // -- if no focus is configured, nothing will happen but trigger
      // the callback that will trigger exposing the camera immediately
    
    if( ! Camera.busy() ) {
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
  

    // if we're in alt i/o as external intervalometer mode...
	  if( altExtInt ) {
			// don't do a pre-output clearance if alt_block is true...
		  if( p_prealt && altBlock )
			return false;
        
			// determine whether or not to fire based on alt_force_shot
		  if( altForceShot == true )
			 return true;
		  else
			 return false;
	  }
  
	// pre--output clearance check
	if(altBeforeDelay >= Camera.interval && !altBlock){  //Camera.interval is less than the altBeforeDelay, go as fast as possible
		return true;
	} else if( (millis() - camera_tm) >= (Camera.interval - altBeforeDelay)  && ! altBlock )
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
		   
		   if( (motor[i].enable() &&  motor[i].planLeadIn() > 0 )){
			   if ((minPlanLead != 0 && motor[i].planLeadIn() < minPlanLead) || (i == 0))
					minPlanLead = motor[i].planLeadIn();
		   }  

	   }
	   
	   //do not move until the minimum plan lead in has based, if planType() == CONT_TL then the plan lead in is in ms
	   	if( ( minPlanLead > 0 && ((camera_fired <= minPlanLead && motor[0].planType()!= CONT_TL) || (motor[0].planType()== CONT_TL && run_time <= minPlanLead))) ) {
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
	 for (int i = 0; i < MOTOR_COUNT; i++){
      if( motor[i].planMoveType  == SMS && motor[i].running() == true )
        return;
	 }

    // no longer running, ok to fire camera

    
    if( ComMgr.master() == true ) {
        // we are a timing master
      Engine.state(ST_CLEAR);    
        
        // if autopause is enabled then pause upon completion of movement
      if( motor[0].autoPause == true || motor[1].autoPause == true || motor[2].autoPause == true ) {
		  //USBSerial.println("Auto pausing!!!");
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




