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

  

 // necessary camera control variables

unsigned int  camera_max_shots = 0;
byte          camera_repeat    = 0;


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
    
  // This callback is called whenever an asynchronous activity begi  ns or ends, and 
  // is able to take some actions as different states are reported out of the
  // camera object.
  //
  // We only care about when certain activities complete, so that's what we look for..
  //
  // Do NOT attempt to use delay() here, or call another camera action directly, as this
  // function is called in an interrupt and can daisy-chain under certain configurations,
  // which can result in unexpected behavior
  
  if( code == OM_CAM_FFIN ) {
    Engine.state(ST_EXP);
  }
  else if( code == OM_CAM_EFIN ) {
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
      // then go ahead and clear for a move
    if( camera_repeat == 0 ) {
      Engine.state(ST_MOVE);
      return;
    }
    
   if( repdone >= camera_repeat ) {
       // we've done all of the repeat cycles
     repdone = 0;
       // clear for moving
     Engine.state(ST_MOVE);
     return;
   }
   
     // trigger another exposure
   repdone++;
   Engine.state(ST_EXP);
}





