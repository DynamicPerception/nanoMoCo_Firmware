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
  moveTimeMinutes = 1;
  moveTimeHours = 0;

  state = USBCONTROLLERMODE_STATE_Setting;
}

float USBControllerMode::CreateDeadzone( float value )
{
  if (abs(value - 0x80) <= DEADZONE)
    return (0x80);
  else
    return (value);
}


#define UI_TICK_RATE     (1000)
#define UI_MODIFIER_QUERY   (PS3CONTROLLER_BUTTON_Select) 

bool USBControllerMode::MonitorButton( PS3Controller_ButtonUsages_t modifierButton, PS3Controller_ButtonUsages_t button, unsigned long *storeMs, uint16_t queryValue )
{
  uint8_t buttonState;
  static uint8_t isMonitoring = false;
  static uint8_t curButtonMonitoring = 0;
  static unsigned long buttonTimer;

  buttonState = PS3CtrlrHost.GetButtonState( button );

  // Only allow one button to be monitored
  if (isMonitoring == false || button == curButtonMonitoring)
  {
    switch (buttonState)
    {
      case PS3CONTROLLER_STATE_Down:
        curButtonMonitoring = button;
        isMonitoring = true;

        buttonTimer = millis();
        ActuatorPulse( 1, 255, 200, 800, 255);
        // LEDPulse( 4, 100, 900, 255);
        break;
      case PS3CONTROLLER_STATE_Up:  
        ActuatorPulseStop();
        isMonitoring = false;
        // Check for button query
        buttonState=PS3CtrlrHost.GetButtonState( modifierButton );
        if(buttonState == PS3CONTROLLER_STATE_Down || buttonState == PS3CONTROLLER_STATE_On)
        {
          // Pulse actuators 
          ActuatorPulse( 1, 255, 200, 800, queryValue);
          return(false);
        }
        //LEDPulseStop(4);
        *storeMs = millis() - buttonTimer;
        return (true);
    }
  }
  return (false);
}


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
    
    // Check for modifiers to play back value or record value by holding down button
    unsigned long buttonTime;
    if(MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_R1, &buttonTime, moveTimeMinutes ))
      moveTimeMinutes = (buttonTime / UI_TICK_RATE);      
    if(MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_R2, &buttonTime, moveTimeHours ))
      moveTimeHours = (buttonTime / UI_TICK_RATE);  
     
    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Start) == PS3CONTROLLER_STATE_Down)
      StartMove();
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

void USBControllerMode::StateWaitToStart( void )
{
  static unsigned long prevTime = 0;
  uint8_t buttonState;
  float new_speed;

  if (!areMotorsRunning())
  {
    // Start the move
    for (uint8_t i = 0; i < MOTOR_COUNT ; i++)
    {
      unsigned long newTime = (60*moveTimeMinutes + 60*60*moveTimeHours)*1000;
      
      motor[i].enable(true);
      motor[i].continuous(true);
      
      byte dir;
      if ((motor[i].stopPos() - motor[i].startPos()) > 0)
        dir = 1;
      else 
        dir = 0;
      
      motor[i].contSpeed( abs(motor[i].stopPos() - motor[i].startPos()) / ((60.0f*moveTimeMinutes + 60.0f*60.0f*moveTimeHours)));
      motor[i].move(dir, abs(motor[i].stopPos() - motor[i].startPos()), newTime,0,0);
      startISR();
    }

    state = USBCONTROLLERMODE_STATE_Wait;
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
  /* LEDPulseState.isOn = true;
   LEDPulseState.LEDNum = LEDNum;
   LEDPulseState.pulsesLeft = nTimes;
   LEDPulseState.offTime = LEDPulseState.offTimeLeft = offMS;
   LEDPulseState.onTime = onMS;
   LEDPulseState.onTimeLeft = 0;
   LEDPulseState.prevUpdateTime = millis();
  // PS3CtrlrHost.SetLED(LEDPulseState.LEDNum, true);*/
  PS3CtrlrHost.SetLEDPulse( LEDNum, onMS, offMS, nTimes );
}

void USBControllerMode::LEDPulseStop( uint8_t LEDNum )
{
  //  LEDPulseState.isOn = false;
  PS3CtrlrHost.SetLED(LEDNum, false);
}

void USBControllerMode::ActuatorPulse( uint8_t ActNum, uint8_t intensity, uint16_t onMS, uint16_t offMS, uint8_t nTimes)
{
  actuatorPulseState.isOn = true;
  actuatorPulseState.ActNum = ActNum;
  actuatorPulseState.intensity = intensity;
  actuatorPulseState.pulsesLeft = nTimes;
  actuatorPulseState.offTime = actuatorPulseState.offTimeLeft = offMS;
  actuatorPulseState.onTime = onMS;
  actuatorPulseState.onTimeLeft = 0;
  actuatorPulseState.prevUpdateTime = millis();
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
  /*
    if (LEDPulseState.isOn)
    {
      timeElapsed = (millis() - LEDPulseState.prevUpdateTime);
      if ( LEDPulseState.offTimeLeft > 0 )
      {
        if (timeElapsed >= LEDPulseState.offTimeLeft)
        {
          LEDPulseState.offTimeLeft = 0;
          LEDPulseState.onTimeLeft = LEDPulseState.onTime;
          PS3CtrlrHost.SetLED(LEDPulseState.LEDNum + 1, true);
        }
        else
          LEDPulseState.offTimeLeft -= timeElapsed;
      }
      else if (LEDPulseState.onTimeLeft > 0 )
      {
        if (timeElapsed >= LEDPulseState.onTimeLeft)
        {
          LEDPulseState.pulsesLeft--;
          if (LEDPulseState.pulsesLeft == 0)
            LEDPulseState.isOn = false;
          else
          {
            PS3CtrlrHost.SetLED( LEDPulseState.LEDNum + 1, false);
            LEDPulseState.offTimeLeft = LEDPulseState.offTime;
          }
        }
        else
          LEDPulseState.onTimeLeft -= timeElapsed;
      }
      LEDPulseState.prevUpdateTime = millis();
    }
  */
  if (actuatorPulseState.isOn)
  {
    timeElapsed = (millis() - actuatorPulseState.prevUpdateTime);
    if ( actuatorPulseState.offTimeLeft > 0 )
    {
      if (timeElapsed >= actuatorPulseState.offTimeLeft)
      {
        actuatorPulseState.offTimeLeft = 0;
        actuatorPulseState.onTimeLeft = actuatorPulseState.onTime;
        if (actuatorPulseState.ActNum == 0)
          PS3CtrlrHost.SetSmallActuator( true, 0xFE);
        else
          PS3CtrlrHost.SetBigActuator(actuatorPulseState.intensity, 0xFE);
      }
      else
        actuatorPulseState.offTimeLeft -= timeElapsed;
    }
    else if (actuatorPulseState.onTimeLeft > 0 )
    {
      if (timeElapsed >= actuatorPulseState.onTimeLeft)
      {
        actuatorPulseState.pulsesLeft--;
        if (actuatorPulseState.pulsesLeft == 0)
          actuatorPulseState.isOn = false;
        else
        {
          actuatorPulseState.offTimeLeft = actuatorPulseState.offTime;
        }
        if (actuatorPulseState.ActNum == 0)
          PS3CtrlrHost.SetSmallActuator( false, 0xFE);
        else
          PS3CtrlrHost.SetBigActuator(0, 0xFE);
      }
      else
        actuatorPulseState.onTimeLeft -= timeElapsed;
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

  if (PS3CtrlrHost.isConnected == true && prevStatus == false)
    PS3CtrlrHost.SetLED(1, true);

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
    case USBCONTROLLERMODE_STATE_WaitToStart:
      StateWaitToStart();
      break;
  }
  IteratePulses();
}

void USBControllerMode::StartMove( void )
{
  // Go to start of move
  sendAllToStart();
  state = USBCONTROLLERMODE_STATE_WaitToStart;
}




