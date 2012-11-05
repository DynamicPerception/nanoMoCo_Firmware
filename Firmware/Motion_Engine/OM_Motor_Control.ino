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

 =========================================
 Motor control functions
 =========================================
 
*/

#define MT_COM_DIR1  50
#define MT_COM_DIR2  100

 // max steps to move during program
 
unsigned long motor_steps_max   = 0;
boolean mt_plan = false;

unsigned long mtpc_steps  = 0;
boolean       mtpc_dir    = false;
unsigned long mtpc_arrive = 0;
unsigned long mtpc_accel  = 0;
unsigned long mtpc_decel  = 0;



void move_motor() {

    // do we revert back to "ready" or "waiting" if there
    // is nothing to do, or block for? - Based on master/slave status
    
   byte continue_state = (ComMgr.master() == true) ? ST_CLEAR : ST_BLOCK;
   
     // motor disabled? Do nothing.
   if( ! Motor.enable() ) {
     Engine.state(continue_state);
     return;
   }
 
 
   if( Motor.continuous() ) {
         // continuous motion mode
      if( ! Motor.running() ) {
         Motor.move( Motor.dir(), 0 );
      }
      Engine.state(continue_state);
   }
   else if( mt_plan == true ) {
       // planned SMS move
     Motor.planRun();
       // block camera while motor is moving
     Engine.state(ST_RUN);
   }
   else if( mtpc == true ) {
       // planned continuous move
     if( mtpc_start == false ) {
       // a planned continuous move has not been started...
       mtpc_start = true;
       Motor.move(mtpc_dir, mtpc_steps, mtpc_arrive, mtpc_accel, mtpc_decel);
     }
     Engine.state(continue_state);
   }
   else if( Motor.steps() == 0 ) {
     // not a planned move and nothing to do
     Engine.state(continue_state);
   }
   else {  
       // move using Motor.steps() and Motor.dir()
     Motor.move();
       // we need to block the camera while the motor is running
     Engine.state(ST_RUN);
   }
   
}


 // callback handler for OMComHandler class to handle trips on the common line 
 // (if we're watching a common line for movement trips)
 
void motor_com_line(unsigned int p_time) {
  if( p_time > MT_COM_DIR1 && p_time < MT_COM_DIR2 )
    Motor.move(0, 1);
  else if( p_time > MT_COM_DIR2 )
    Motor.move(1, 1);
}


      





