
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
Key frame functions
========================================

*/

void kfNext() {
	//uint8_t frame = input_serial_buffer[0];
	uint8_t frame = key_frames;

	for (byte i = 0; i < MOTOR_COUNT; i++) {
		motor[i].keyDest(frame, motor[i].currentPos());
		if (usb_debug & DB_GEN_SER){
			USBSerial.println("Motor pos: ");
			USBSerial.println(motor[i].keyDest(frame));
		}
	}

	if (usb_debug & DB_GEN_SER){
		USBSerial.print("Key frame number ");
		USBSerial.print(key_frames);
		USBSerial.println(" set!");
	}

	key_frames++;
}

void kfSet(byte p_input) {
	key_move = p_input;
	// When entering key frame mode, reset the number of key frames
	if (key_move) {
		key_frames = 0;
		current_frame = 0;
		if (usb_debug & DB_GEN_SER)
			USBSerial.println("Resetting key frame and current frame counts");
	}

	//USBSerial.print("Key move status: ");
	//USBSerial.println(key_move);
}