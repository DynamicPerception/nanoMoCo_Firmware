/*

ControllerMode

See dynamicperception.com for more information


(c) 2016 B. Wiggins / Dynamic Perception LLC

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
#include "PS3ControllerHost.h"

#define DEADZONE (10.0)
USBControllerMode::USBControllerMode(void)
{
}

float USBControllerMode::CreateDeadzone( float value )
{
  if (abs(value-0x80)<=DEADZONE)
    return(0x80);
  else
    return(value);
}
  static LUFAStat prevStatus=STAT_NOTCONNECTED;
    static float prevLeftXVelocity=0;
  static float prevLeftYVelocity=0;
  static float prevRightXVelocity=0;
  static float prevRightYVelocity=0;
  
void USBControllerMode::PerFrameTask( void )
{
  unsigned long prevTime;
  
  if(millis()-prevTime < 10)
  {
    return;
  }
  prevTime = millis();
  
  // Per frame driver processing  
  PS3CtrlrHost.USBTask();
  USB_USBTask();

  
  // Display connect/Disconnects 

  if(PS3CtrlrHost.status() == STAT_NOTCONNECTED && prevStatus == STAT_CONNECTED)
  {  
    debug.functln("Ps3Disc");
  }
  else if (PS3CtrlrHost.status() == STAT_CONNECTED && prevStatus == STAT_NOTCONNECTED)
  {
    debug.functln("Ps3Conn");
  }
  
  prevStatus=PS3CtrlrHost.status();
  
  if (PS3CtrlrHost.IsConnected())
  {
    // Stop drifting when idle
    float leftXVelocity=CreateDeadzone(PS3CtrlrHost.GetLeftStickX());
    float leftYVelocity=CreateDeadzone(PS3CtrlrHost.GetLeftStickY());
    float rightXVelocity=CreateDeadzone(PS3CtrlrHost.GetRightStickX());
    float rightYVelocity=CreateDeadzone(PS3CtrlrHost.GetRightStickY());

      // Map PS3 sticks to first four axis
    if(leftXVelocity!=prevLeftXVelocity) setJoystickSpeed(0, ((leftXVelocity-128.0f)/128.0f)* mot_max_speed );
    if(leftYVelocity!=prevLeftYVelocity) setJoystickSpeed(2, ((leftYVelocity-128.0f)/128.0f)* mot_max_speed );
    if(rightXVelocity!=prevRightXVelocity) setJoystickSpeed(1, ((rightXVelocity-128.0f)/128.0f)* mot_max_speed );
    if(rightYVelocity!=prevRightYVelocity) setJoystickSpeed(3, ((rightYVelocity-128.0f)/128.0f)* mot_max_speed );
    
    prevLeftXVelocity = leftXVelocity;
    prevLeftYVelocity = leftYVelocity;
    prevRightXVelocity = rightXVelocity;
    prevRightYVelocity = rightYVelocity;
  }
  else
  {
    if(prevLeftXVelocity!=0) setJoystickSpeed(0, 0 );
    if(prevLeftYVelocity!=0) setJoystickSpeed(2, 0 );
    if(prevRightXVelocity!=0) setJoystickSpeed(1, 0 );
    if(prevRightYVelocity!=0) setJoystickSpeed(3, 0 );
    
    prevLeftXVelocity = 0;
    prevLeftYVelocity = 0;
    prevRightXVelocity = 0;
    prevRightYVelocity = 0;
  }
}
