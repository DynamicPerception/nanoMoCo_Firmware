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
 Motor traveling functions
 =========================================

 
 */
 
 
void sendAllToStart() {

	// If any of the motors are currently running, don't start the motors moving
	for (byte i = 0; i < MOTOR_COUNT; i++) {
		if (motor[i].running()) {
			return;
		}
	}

	for (byte i = 0; i < MOTOR_COUNT; i++) {
		sendToStart(i);
	}
}

void sendToStart(uint8_t p_motor) {

	// Determine whether this move will induce backlash that needs to be taken up before starting the program move
	uint8_t program_dir = (motor[p_motor].stopPos() - motor[p_motor].startPos()) > 0 ? 1 : 0;
	uint8_t this_move_dir = (motor[p_motor].startPos() - motor[p_motor].currentPos()) > 0 ? 1 : 0;

	if (program_dir != this_move_dir)
		motor[p_motor].programBackCheck(true);
	else
		motor[p_motor].programBackCheck(false);

	// Move at the maximum motor speed	
	motor[p_motor].ms(4);
    motor[p_motor].contSpeed(motor[p_motor].maxSpeed());

	// Start the move
	motor[p_motor].moveToStart();
	startISR();
	motor[p_motor].setSending(true);
}

void sendToStop(uint8_t p_motor){	
	// Move at the maximum motor speed		
	motor[p_motor].ms(4);
    motor[p_motor].contSpeed(motor[p_motor].maxSpeed());
	motor[p_motor].moveToStop();
	startISR();
	motor[p_motor].setSending(true);
}

void sendTo(uint8_t p_motor, long p_pos){
	sendTo(p_motor, p_pos, false);
}

void sendTo(uint8_t p_motor, long p_pos, boolean kf_move){
	
	// When not in Graffik Mode (i.e. App mode), use the lowest microsteps	
	if (!kf_move){
		motor[p_motor].ms(4);
		// Adjust the send location to match new microsteps
		p_pos *= ((float)motor[p_motor].ms() / (float)motor[p_motor].lastMs());
	}

	// Move at the maximum motor speed
	debug.funct("Sending motor ");
	debug.funct(p_motor);
	debug.funct(" to position ");
	debug.functln(p_pos);
    motor[p_motor].contSpeed(motor[p_motor].maxSpeed());
	motor[p_motor].moveTo(p_pos, true);
	debug.funct("Speed: ");
	debug.functln(motor[p_motor].contSpeed());
	debug.funct("Continuous: ");
	debug.functln(motor[p_motor].continuous());
	startISR();
	if (!kf_move)
		motor[p_motor].setSending(true);
}