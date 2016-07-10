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

void USBControllerMode::init(void)
{
  PS3CtrlrHost.init();
  prevStatus = false;
  prevLeftXVelocity = 0;
  prevLeftYVelocity = 0;
  prevRightXVelocity = 0;
  prevRightYVelocity = 0;

  LEDPulseState.isOn = false;
  actuatorPulseState.isOn = false;

  state = USBCONTROLLERMODE_STATE_Setting;
}

float USBControllerMode::CreateDeadzone( float value )
{
  if (abs(value - 0x80) <= DEADZONE)
    return (0x80);
  else
    return (value);
}


#define UI_TICK_RATE (1000)
/*
bool USBControllerMode::MonitorButton( PS3Controller_ButtonUsages_t button, unsigned long *storeMs )
{
  uint8_t buttonState;
  buttonState = PS3CtrlrHost.GetButtonState( button );

  switch (buttonState)
  {
    case PS3CONTROLLER_STATE_Down:
      buttonTimers[button - 1] = millis();
      ActuatorPulse( 1, 255, UI_TICK_RATE / 2, UI_TICK_RATE / 2, 255);
      LEDPulse( 4, UI_TICK_RATE / 2, UI_TICK_RATE / 2, 255);
      break;
    case PS3CONTROLLER_STATE_Up:
      ActuatorPulseStop( 1 );
      LEDPulseStop( 4 );
      *storeMs = millis() - buttonTimers[button - 1];
      return (true);
      break;
    case PS3CONTROLLER_STATE_Off:
      return (false);
      break;
  }
}
*/
void USBControllerMode::StateSetting( void )
{
  if (PS3CtrlrHost.isConnected)
  {
    // Stop drifting when idle
    float leftXVelocity = CreateDeadzone(PS3CtrlrHost.GetLeftStickX());
    float leftYVelocity = CreateDeadzone(PS3CtrlrHost.GetLeftStickY());
    float rightXVelocity = CreateDeadzone(PS3CtrlrHost.GetRightStickX());
    float rightYVelocity = CreateDeadzone(PS3CtrlrHost.GetRightStickY());

    // Map PS3 sticks to first four axis
    if (leftXVelocity != prevLeftXVelocity) setJoystickSpeed(0, ((leftXVelocity - 128.0f) / 128.0f)* mot_max_speed );
    if (leftYVelocity != prevLeftYVelocity) setJoystickSpeed(3, ((leftYVelocity - 128.0f) / 128.0f)* mot_max_speed );
    if (rightXVelocity != prevRightXVelocity) setJoystickSpeed(1, -((rightXVelocity - 128.0f) / 128.0f)* mot_max_speed );
    if (rightYVelocity != prevRightYVelocity) setJoystickSpeed(2, -((rightYVelocity - 128.0f) / 128.0f)* mot_max_speed );

    prevLeftXVelocity = leftXVelocity;
    prevLeftYVelocity = leftYVelocity;
    prevRightXVelocity = rightXVelocity;
    prevRightYVelocity = rightYVelocity;

    uint8_t buttonState;

    // Map DPAD Left/Right to Start/Stop points
    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Left) == PS3CONTROLLER_STATE_Down)
    {
      //PS3CtrlrHost.SetBigActuator(255,128);
      ActuatorPulse( 0, 255, 150, 100, 1);
      motor[0].startPos(motor[0].currentPos());  // Set start point on axis 0
      motor[1].startPos(motor[1].currentPos());  // Set start point on axis 1
      motor[2].startPos(motor[2].currentPos());  // Set start point on axis 2
    }
    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Right) == PS3CONTROLLER_STATE_Down)
    {
      //LEDPulse( 4, 500, 250, 10);
      ActuatorPulse( 0, 255, 150, 100, 2);
      motor[0].stopPos(motor[0].currentPos());  // Set stop point on axis 0
      motor[1].stopPos(motor[1].currentPos());  // Set stop point on axis 1
      motor[2].stopPos(motor[2].currentPos());  // Set stop point on axis 2
    }

    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Cross) == PS3CONTROLLER_STATE_Down)
    {
      // Go to start of move
      sendAllToStart();
      state = USBCONTROLLERMODE_STATE_Wait;
    }
    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Circle) == PS3CONTROLLER_STATE_Down)
    {
      // Go to end of move
      sendAllToStop();
      state = USBCONTROLLERMODE_STATE_Wait;
    }

    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_R3) == PS3CONTROLLER_STATE_Down)
    {
      int freeMem = freeMemory();
      char szTemp[16];
      sprintf(szTemp, "FREEMEM: %d", freeMem);
      debug.functln(szTemp);
    }

    /*   unsigned long buttonTime;
       if(MonitorButton( PS3CONTROLLER_BUTTON_R1, &buttonTime ))
       {
         moveTimeMinutes = (uint8_t) (buttonTime/UI_TICK_RATE);
         //char szTemp[16];
         //sprintf(szTemp, "Minutes: %d", moveTimeMinutes);
         //debug.functln(szTemp);
       }

       if(MonitorButton( PS3CONTROLLER_BUTTON_R2, &buttonTime ))
       {
         moveTimeHours = (uint8_t) (buttonTime/UI_TICK_RATE);
         //char szTemp[16];
         //sprintf(szTemp, "Hours: %d", moveTimeHours);
         //debug.functln(szTemp);
       }
      */
  }
  else
  {
    if (prevLeftXVelocity != 0) setJoystickSpeed(0, 0 );
    if (prevLeftYVelocity != 0) setJoystickSpeed(2, 0 );
    if (prevRightXVelocity != 0) setJoystickSpeed(1, 0 );
    if (prevRightYVelocity != 0) setJoystickSpeed(3, 0 );

    prevLeftXVelocity = 0;
    prevLeftYVelocity = 0;
    prevRightXVelocity = 0;
    prevRightYVelocity = 0;
  }
}

#define KILL_TIMER_TIME (1000)

void USBControllerMode::StateWait( void )
{
  static unsigned long prevTime = 0;
  uint8_t buttonState;

  if (!areMotorsRunning())
  {
    state = USBCONTROLLERMODE_STATE_Setting;
    return;
  }

  buttonState = PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_L3);
  if ( buttonState == PS3CONTROLLER_STATE_Down)
    prevTime = millis();

  if ( (buttonState == PS3CONTROLLER_STATE_On) && ((millis() - prevTime) > KILL_TIMER_TIME))
  {
    stopAllMotors();
    state = USBCONTROLLERMODE_STATE_Setting;
  }
}

void USBControllerMode::LEDPulse( uint8_t LEDNum, uint16_t onMS, uint16_t offMS, uint8_t nTimes)
{
  LEDPulseState.isOn = true;
  LEDPulseState.LEDNum = LEDNum;
  LEDPulseState.pulsesLeft = nTimes;
  LEDPulseState.onTime = LEDPulseState.onTimeLeft = onMS;
  LEDPulseState.offTime = offMS;
  LEDPulseState.offTimeLeft = 0;
  LEDPulseState.prevUpdateTime = millis();
  PS3CtrlrHost.SetLED(LEDPulseState.LEDNum, true);
}

void USBControllerMode::LEDPulseStop( void )
{
  LEDPulseState.isOn = false;
  PS3CtrlrHost.SetLED(LEDPulseState.LEDNum, false);
}

void USBControllerMode::ActuatorPulse( uint8_t ActNum, uint8_t intensity, uint16_t onMS, uint16_t offMS, uint8_t nTimes)
{
  actuatorPulseState.isOn = true;
  actuatorPulseState.ActNum = ActNum;
  actuatorPulseState.intensity = intensity;
  actuatorPulseState.pulsesLeft = nTimes;
  actuatorPulseState.onTime = actuatorPulseState.onTimeLeft = onMS;
  actuatorPulseState.offTime = offMS;
  actuatorPulseState.offTimeLeft = 0;
  actuatorPulseState.prevUpdateTime = millis();

  if (actuatorPulseState.ActNum == 0)
    PS3CtrlrHost.SetSmallActuator(true, 0xFE);

  if (actuatorPulseState.ActNum == 1)
    PS3CtrlrHost.SetBigActuator(actuatorPulseState.intensity, 0xFE);
}

void USBControllerMode::ActuatorPulseStop( void )
{
  actuatorPulseState.isOn = false;
  if (actuatorPulseState.ActNum == 0)
    PS3CtrlrHost.SetSmallActuator(false, 0x00);

  if (actuatorPulseState.ActNum == 1)
    PS3CtrlrHost.SetBigActuator(0, 0x00);
}

// Only pulsing one button/LED at a time is supported
void USBControllerMode::IteratePulses( void )
{
  unsigned long timeElapsed;
  uint8_t i;


  if (LEDPulseState.isOn)
  {
    timeElapsed = (millis() - LEDPulseState.prevUpdateTime);
    if ( LEDPulseState.onTimeLeft > 0 )
    {
      if (timeElapsed >= LEDPulseState.onTimeLeft)
      {
        LEDPulseState.onTimeLeft = 0;
        LEDPulseState.offTimeLeft = LEDPulseState.offTime;
        PS3CtrlrHost.SetLED(LEDPulseState.LEDNum + 1, false);
      }
      else
        LEDPulseState.onTimeLeft -= timeElapsed;
    }
    else if (LEDPulseState.offTimeLeft > 0 )
    {
      if (timeElapsed >= LEDPulseState.offTimeLeft)
      {
        LEDPulseState.pulsesLeft--;
        if (LEDPulseState.pulsesLeft == 0)
          LEDPulseState.isOn = false;
        else
        {
          PS3CtrlrHost.SetLED( LEDPulseState.LEDNum + 1, true);
          LEDPulseState.onTimeLeft = LEDPulseState.onTime;
        }
      }
      else
        LEDPulseState.offTimeLeft -= timeElapsed;
    }
    LEDPulseState.prevUpdateTime = millis();
  }

  if (actuatorPulseState.isOn)
  {
    timeElapsed = (millis() - actuatorPulseState.prevUpdateTime);
    if ( actuatorPulseState.onTimeLeft > 0 )
    {
      if (timeElapsed >= actuatorPulseState.onTimeLeft)
      {
        actuatorPulseState.onTimeLeft = 0;
        actuatorPulseState.offTimeLeft = actuatorPulseState.offTime;
        if (actuatorPulseState.ActNum == 0)
          PS3CtrlrHost.SetSmallActuator( false, 0xFE);
        else
          PS3CtrlrHost.SetBigActuator(0, 0xFE);
      }
      else
        actuatorPulseState.onTimeLeft -= timeElapsed;
    }
    else if (actuatorPulseState.offTimeLeft > 0 )
    {
      if (timeElapsed >= actuatorPulseState.offTimeLeft)
      {
        actuatorPulseState.pulsesLeft--;
        if (actuatorPulseState.pulsesLeft == 0)
          actuatorPulseState.isOn = false;
        else
        {
          actuatorPulseState.onTimeLeft = actuatorPulseState.onTime;
          if (actuatorPulseState.ActNum == 0)
            PS3CtrlrHost.SetSmallActuator( true, 0xFE);
          else
            PS3CtrlrHost.SetBigActuator(actuatorPulseState.intensity, 0xFE);
        }
      }
      else
        actuatorPulseState.offTimeLeft -= timeElapsed;
    }

    actuatorPulseState.prevUpdateTime = millis();
  }
}


void USBControllerMode::CtrlrTask( void )
{
  unsigned long prevTime;

  if (millis() - prevTime < 10)
  {
    return;
  }
  prevTime = millis();

  // Per frame driver processing
  PS3CtrlrHost.USBTask();
  USB_USBTask();

  // Display connect/Disconnects
  if (PS3CtrlrHost.isConnected == false && prevStatus == true)
  {
    //debug.functln("Ps3Disc");
  }
  else if (PS3CtrlrHost.isConnected == true && prevStatus ==false)
  {
    PS3CtrlrHost.SetLED(1, true);
    //debug.functln("Ps3Conn");
  }

  prevStatus = PS3CtrlrHost.isConnected;

  // Run USB Controller State machine
  switch (state)
  {
    case USBCONTROLLERMODE_STATE_Setting:
      StateSetting();
      break;
    case USBCONTROLLERMODE_STATE_Wait:
      StateWait();
      break;
  }
  IteratePulses();
}

void USBControllerMode::StartMove( void )
{
  // Go to start of move
  for (uint8_t i = 0 ; i < MOTOR_COUNT ; i++)
    sendToStart(i);
}




