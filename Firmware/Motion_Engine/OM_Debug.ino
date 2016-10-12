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
  Debug functions
  ========================================
  
*/

/*
byte setDebugOutput(byte p_setting)

Sets the USB debug output flags. This allows for turning on and off of specific debug message types.

@param 
p_setting is an 8-bit number where each bit is a debug flag. A bit will toggle the corresponding bit
in the usb_debug variable to its opposite state. Set only one bit at a time to avoid conflicting on/off requests.

*/

byte setDebugOutput(byte p_setting) {
    byte setting = debug.getState();
    // If the specified flag is already on, turn it off, otherwise turn it on.
    if (setting & p_setting)
        setting &= ~p_setting;
    else
        setting |= p_setting;
    debug.setState(setting);

    // Set debug states for libraries
    bool motor_debug = false;
    bool camera_debug = false;
    if (setting & DebugClass::DB_FUNCT)
        camera_debug = true;
    if (setting & DebugClass::DB_MOTOR)
        motor_debug = true;
    OMCamera::debugOutput(camera_debug);
    OMMotorFunctions::debugOutput(motor_debug);

    // Return the full USB debug flag byte
    return setting;
}

/*
void debugOn() / void debugOff()

Sets the debug LED on or off, respectively

*/

void debugOn() {  
  digitalWrite(DEBUG_PIN, HIGH);
  debug_LED = true;
}

void debugOff() {  
  digitalWrite(DEBUG_PIN, LOW);
  debug_LED = false;
}

// Toggles the state of the debug LED
void debugToggle(){
    if(debug_LED)
        debugOff();
    else
        debugOn();
}

void selfDiagnostic() {
    // Check camera trigger
    for (byte i = 0; i < 2; i++) {
        Camera.expose();
        delay(1000);
    }

    // Enable and sleep all the motors so all LEDs are off
    for (byte i = 0; i < MOTOR_COUNT; i++){
        motor[i].enable(true);
        motor[i].sleep(true);
    }

    // Check the motor attachment
    uint8_t motor_attach = checkMotorAttach();
    delay(250);

    // Report via enable lights which motors were detected
    for (byte i = 0; i < MOTOR_COUNT; i++){
        // If a motor is detected, turn on the LED by turning off sleep mode
        if ((motor_attach >> i) & 1){
            motor[i].sleep(false);
            delay(250);
            motor[i].sleep(true);
            delay(250);
        }
        else
            delay(1000);
    }

    // Turn sleep mode back off for all motor channels
    for (byte i = 0; i < MOTOR_COUNT; i++)
        motor[i].sleep(false);

    // Move motors forward
    unsigned long _time = millis();
    int _speed = 4000;
    int _seconds = 3;
    for (byte i = 0; i < MOTOR_COUNT; i++){
        motor[i].contSpeed(_speed);
        motor[i].continuous(false);
        motor[i].move(0, (_speed * _seconds)); // Determine the distanced based upon the speed and desired seconds above.
    }
    startISR();
    while (motor[0].running() || motor[1].running() || motor[2].running()){
        // Do nothing until all motors have stopped
    }
    for (byte i = 0; i < MOTOR_COUNT; i++)
        motor[i].move(1, (_speed * _seconds));
    startISR();
}
    
  
