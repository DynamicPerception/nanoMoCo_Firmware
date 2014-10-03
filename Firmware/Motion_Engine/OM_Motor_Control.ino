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

const byte MT_COM_DIR1 = 50;
const byte MT_COM_DIR2 = 100;



void move_motor() {

    // do we revert back to "ready" or "waiting" if there
    // is nothing to do, or block for? - Based on master/slave status
    
   byte continue_state = (ComMgr.master() == true) ? ST_CLEAR : ST_BLOCK;
   
     // motor disabled? Do nothing.
   if( ! motor[0].enable() && !motor[1].enable() && !motor[2].enable() ) {
     Engine.state(continue_state);
     return;
   }
   
   for(int i = 0; i < MOTOR_COUNT; i++){
	   //only check the motors that are enabled
	   if( motor[i].enable()){
		   if( motor[i].continuous() ) {
			   // continuous motion mode
			   if( ! motor[i].running() ) {
				   motor[i].move( motor[i].dir(), 0 );
			   }
			   Engine.state(continue_state);
		   }
		   else if( motor[i].mt_plan == true ) {
			   // planned SMS move
			   motor[i].planRun();
			   // block camera while motor is moving
			   Engine.state(ST_RUN);
		   }
		   else if( motor[i].mtpc == true ) {
			   // planned continuous move
			   if( motor[i].mtpc_start == false ) {
				   // a planned continuous move has not been started...
				   motor[i].mtpc_start = true;
				   motor[i].move(motor[i].mtpc_dir, motor[i].mtpc_steps, motor[i].mtpc_arrive, motor[i].mtpc_accel, motor[i].mtpc_decel);
			   }
			   Engine.state(continue_state);
		   }
		   else if( motor[i].steps() == 0 ) {
			   // not a planned move and nothing to do
			   Engine.state(continue_state);
		   }
		   else {
			   // move using Motor.steps() and Motor.dir()
			   motor[i].move();
			   // we need to block the camera while the motor is running
			   Engine.state(ST_RUN);
		   } 
	   }

   }
   
   
   //Start interrupt service rotuine to start motors moving
   startISR();
 
 
  
   
}


 // callback handler for OMComHandler class to handle trips on the common line 
 // (if we're watching a common line for movement trips)
 
void motor_com_line(unsigned int p_time) {
  if( p_time > MT_COM_DIR1 && p_time < MT_COM_DIR2 ){
    motor[0].move(0, 1);
	motor[1].move(0, 1);
	motor[2].move(0, 1);
  }
  else if( p_time > MT_COM_DIR2 ){
    motor[0].move(1, 1);
    motor[1].move(1, 1);
    motor[2].move(1, 1);
  }
}


      





