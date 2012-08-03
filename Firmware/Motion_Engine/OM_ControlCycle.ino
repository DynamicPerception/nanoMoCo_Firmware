/*


OpenMoco

Time-lapse Core Engine

See www.openmoco.org for more information


(c) 2008-2011 C.A. Church / Dynamic Perception LLC

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




unsigned long  camera_tm        = 0;
unsigned long  camera_delay     = 1;
boolean        camera_on        = false;
unsigned long  motor_delay      = 0;



void setupControlCycle() {

 Engine.setHandler(ST_CLEAR, cycleCamera);
 Engine.setHandler(ST_MOVE, cycleClearToMove);
 Engine.setHandler(ST_RUN, cycleCheckMotor);
 Engine.setHandler(ST_EXP, camExpose);
 Engine.setHandler(ST_WAIT, camWait);
}



void cycleCamera() {

    // stop program if max shots exceeded    
  if( camera_max_shots > 0  && camera_fired >= camera_max_shots ) {
           // stop program running
      running = false;
      Engine.state(ST_BLOCK);
      return;
  }
  


    // if enough time has passed, and we're ok to take an exposure
    // note: for slaves, we only get here by a master signal, so we don't check interval timing
  
  if( ComMgr.master() == false || ( millis() - camera_tm ) >= camera_delay  ) {
    
    
            // skip camera actions if camera disabled  
      if( ! camera_on ) {
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
      camera_tm = millis();  
      Camera.focus();
    } 
    
  }
  
}

void cycleClearToMove() {
       // signal any slaves that they're ok to proceed, if master
       ComMgr.masterSignal();
       
       // do not move if a motor delay is programmed...
      if( motor_delay > 0 && run_time < motor_delay ) {
        Engine.state(ST_CLEAR);
        return;
      }        
      
         // ok to run motors, if needed
      move_motor(); 
}


void cycleCheckMotor() {
         // still running
     
     // do not block on continuous motion of any sort
      if( Motor.continuous() == false && mtpc == false && Motor.running() == true )
        return;


    // no longer running, ok to fire camera

    
    if( ComMgr.master() == true ) {
        // we are a timing master
      Engine.state(ST_CLEAR);        
    }
    else { 
        // we are a slave - block until next slaveClear signal
      Engine.state(ST_BLOCK);
     }
}




