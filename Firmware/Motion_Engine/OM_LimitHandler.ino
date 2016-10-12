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


// debounce threshold time
const int ALT_TRIG_THRESH  = 750;
  



//Limit switch inputs (0 = detached, 1 = attached RISING, 2 = attached FALLING, 3 = attached CHANGE)
byte                     ring = 0;
byte                      tip = 0;

//Pins for the I/O
const byte           AUX_RING = 24;             //PD0
const byte            AUX_TIP = 25;             //PD1



//timer for the I/O interrupt
unsigned long trigLast = millis();



void limitSwitchAttach(byte p_altDirection) { 
    
    
    switch(p_altDirection){
        case 0: 
            altDirection = RISING;
            break;
        case 1:
            altDirection = FALLING;
            break;
        case 2:
            altDirection = CHANGE;
            break;
        default:
            break;
    }
}


 /** Alt I/O Setup 
 
*/
void altSetup() {
  altConnect(0 ,  altInputs[0]);
  altConnect(1,  altInputs[1]); 

    // check if any inputs are set to ext intervalometer
  uint8_t doExt = false;
  
  for( byte i = 0; i < 2; i++ )
    if( altInputs[i] == ALT_EXTINT )
      doExt = true;
      
  altExtInt = doExt;
  
}

/** Handler For Alt I/O Action Trigger

 Given a particular Alt I/O line being triggered in input mode, this function
 debounces and executes any required action for an Alt I/O line.
 
 @param p_which
 The I/O line that triggered, 0 or 1.
 
 @author
 C. A. Church
 */

void altHandler(byte p_which) {
    
  if((millis() - trigLast) >= ALT_TRIG_THRESH ) {
    
    trigLast = millis();
    
    if( altInputs[p_which] == ALT_START) 
        startProgram();
    else if( altInputs[p_which] == ALT_STOP_MOTORS && running == true)
        stopAllMotors();
    else if( altInputs[p_which] == ALT_STOP)
        stopProgram();
    else if( altInputs[p_which] == ALT_TOGGLE) {
        if( running )
          stopProgram();
        else
          startProgram();
    }
    else if( altInputs[p_which] == ALT_EXTINT ) {
        
        debug.serln("External trigger detected");
        
        // set camera ok to fire
        altForceShot = true;
          // do not clear the state, as we may be in the middle of a move
          // when a trigger is received! (or some other activity, for that matter)
        // Engine.state(ST_CLEAR);
    }
    else if( altInputs[p_which] == ALT_DIR) {
        motor[0].dir( !motor[0].dir() );
        motor[1].dir( !motor[1].dir() );
        motor[2].dir( !motor[2].dir() );
    } else if (altInputs[p_which] == ALT_SET_HOME){
        stopAllMotors();
        motor[0].homeSet();
        motor[1].homeSet();
        motor[2].homeSet();
    } else if (altInputs[p_which] == ALT_SET_END){
        stopAllMotors();
        motor[0].maxSteps((motor[0].currentPos()));
        motor[1].maxSteps((motor[1].currentPos()));
        motor[2].maxSteps((motor[2].currentPos()));
    }
        
  } //end if millis...
}

/** Handler for ISR One */ 
void altISROne() {
  altHandler(0);
}

/** Handler for ISR Two */
void altISRTwo() {
  altHandler(1);
}

/** Connect (or Disconnect) an Alt I/O Line

 This function attches or detaches the required interrupt
 given an I/O line and a mode.
 
 @param p_which
 Which I/O, 0 or 1
 
 @param p_mode
 A valid altMode 
 
 @author
 C. A. Church
 */

void altConnect(byte p_which, byte p_mode) {
  
  
    altInputs[p_which] = p_mode;
    //sets the pin affected
    byte pin = AUX_RING;
    if (p_which == 1)
        pin = AUX_TIP;
  
    if( p_which == 0) {
        detachInterrupt(2);
    } else if ( p_which == 1) {
        detachInterrupt(3);
    } 

    // disable the input?
 
    if( p_mode == ALT_OFF ) {
        pinMode(pin, INPUT);
        return;
    }
  
    // only attach interrupts for input modes
    if( p_mode != ALT_OUT_BEFORE && p_mode != ALT_OUT_AFTER ) {
        // set pin as input
        pinMode(pin, INPUT);
        // enable pull-up resistor
        digitalWrite(pin, HIGH);
  
        // regarding 6 and 7 below - don't ask me.. ask who ever did that wierd order in WInterrupts.c
        switch( p_which ) {
            case 0: //sets interrupt attached to ring
                attachInterrupt(2, altISROne, altDirection);
                break;
            case 1: //sets interrupt attached to tip
                attachInterrupt(3, altISRTwo, altDirection);
                break;
        }
    } else {    
        // it's an output mode    
        pinMode(pin, OUTPUT);
        digitalWrite(pin, ! altOutTrig);    
    }
 
  
}    
  
 
/** Trigger Outputs

 Triggers all configured outputs for the given mode, for the given length
 of time.  Blocks state engine while output is triggered, and registers callback
 to bring output down.
 
 @param p_mode
 Either ALT_TRIG_A or ALT_TRIG_B, specifying after or before types
 
 @author
 C. A. Church
 
 */
 

void altOutStart(byte p_mode) {
  
  uint8_t altStarted = false;
  
  unsigned int adelay = p_mode == ALT_OUT_BEFORE ? altBeforeMs  : altAfterMs;
  
  for(byte i = 0; i < 2; i++) {
       if( p_mode == altInputs[i]) {
         
           // note that alt 3 is on a different register..
          if( i == 0 )
            digitalWrite(AUX_RING, altOutTrig); 
          else
            digitalWrite(AUX_TIP, altOutTrig);
            
          altStarted = true;
       }
  }
    
  if( altStarted ) {
    MsTimer2::set(adelay, altOutStop);
    MsTimer2::start();
    Engine.state(ST_BLOCK);
  }
  else if( p_mode == ALT_OUT_BEFORE ) {
      //clear to shoot
    Engine.state(ST_CLEAR);
  }
  else {
      // clear to move
    Engine.state(ST_MOVE);
  }
  
}

/** Bring Down any Triggered Outputs 

 @author 
 C. A. Church
 */
 
void altOutStop() {
 
  MsTimer2::stop();
  
   
  for(byte i = 0; i < 2; i++) {
       if( ALT_OUT_BEFORE == altInputs[i] || ALT_OUT_AFTER == altInputs[i] ) {
           // note that alt 3 is on a different register..
          if( i == 0 )
            digitalWrite(AUX_RING, ! altOutTrig); 
          else
            digitalWrite(AUX_TIP, ! altOutTrig);
       }
  }
  
  // set correct state to either clear to fire, or clear to move
  
  if( altBlock == ALT_OUT_BEFORE )
  {
    Engine.state(ST_CLEAR);
  }
  else
  {
    Engine.state(ST_MOVE);
  }
    
}

