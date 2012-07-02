
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

 =========================================
 Motor control functions
 =========================================
 
*/


 // max steps to move during program
 
unsigned long motor_steps_max   = 0;
boolean mt_plan = false;

void move_motor() {

    // do we revert back to "ready" or "waiting" if there
    // is nothing to do, or block for? - Based on master/slave status
    
   byte continue_state = (ComMgr.master() == true) ? ST_CLEAR : ST_BLOCK;
   
   if( ! Motor.enable() ) {
     Engine.state(continue_state);
     return;
   }
 
   static unsigned int motor_cont_spd_was = 0;
   
  
   if( Motor.continuous() ) {
       // continuous motion mode
    if( ! Motor.running() ) {
       Motor.move( Motor.dir(), 0 );
    }
    Engine.state(continue_state);
    return;
   }
   
 
    // not in continuous motion mode

   if( mt_plan == true ) {
       // planned move
     Motor.planRun();
       // block camera while motor is moving
     Engine.state(ST_RUN);
     return;
   }
     
   if( Motor.steps() == 0 ) {
     // not a planned move and nothing to do
     Engine.state(continue_state);
     return;
   }
     
     // move using Motor.steps() and Motor.dir()
   Motor.move();
     // we need to block the camera while the motor is running
   Engine.state(ST_RUN);
}




      





