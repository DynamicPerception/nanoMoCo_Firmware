/*

ControllerUI

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

USBControllerUI USBCtrlrUI;

#define UI_DEADZONE (10.0)
#define UI_TICK_RATE     (700)
#define UI_MODIFIER_QUERY   (PS3CONTROLLER_BUTTON_Select)
#define KILL_TIMER_TIME (2000)


USBControllerUI::USBControllerUI(void)
{
}

/** Initialize

  Initialize USBController UI

*/
void USBControllerUI::init(void)
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
  accelTime = 0;
  decelTime = 0;
  isShotRunning = false;
  isContinuous = true;
  exposureTimeS = CAM_DEFAULT_EXP;
  exposureWaitS = CAM_DEFAULT_WAIT;
  exposureTimeDS = 1;
  exposureWaitDS = 0;

  state = USBCONTROLLERUI_STATE_Setting;
}

/** UI Task

  Perform per frame tasks for UI

*/
void USBControllerUI::UITask( void )
{
  uint32_t prevTime;

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
    case USBCONTROLLERUI_STATE_Setting:
      StateSetting();
      break;
    case USBCONTROLLERUI_STATE_Wait:
      StateWait();
      break;
    case USBCONTROLLERUI_STATE_WaitToStart:
      StateWaitToStart();
      break;
  }

  IteratePulses();
}

/** Setting State

  In this UI state, information regarding the move can be input including
  start/stop points and timers.

*/
void USBControllerUI::StateSetting( void )
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
      ActuatorPulse( 0, 255, 150, 100, 1);
      motor[0].startPos(motor[0].currentPos());  // Set start point on axis 0
      motor[1].startPos(motor[1].currentPos());  // Set start point on axis 1
      motor[2].startPos(motor[2].currentPos());  // Set start point on axis 2
    }
    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Right) == PS3CONTROLLER_STATE_Down)
    {
      ActuatorPulse( 0, 255, 150, 100, 2);
      motor[0].stopPos(motor[0].currentPos());  // Set stop point on axis 0
      motor[1].stopPos(motor[1].currentPos());  // Set stop point on axis 1
      motor[2].stopPos(motor[2].currentPos());  // Set stop point on axis 2
    }
    
    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_R3) == PS3CONTROLLER_STATE_Down)
    {
      // Go to start of move
      sendAllToStart();
      state = USBCONTROLLERUI_STATE_Wait;
    }
   /*     if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Circle) == PS3CONTROLLER_STATE_Down)
        {
          // Go to end of move
          sendAllToStop();
          state = USBCONTROLLERUI_STATE_Wait;
        }
    */
    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_L3) == PS3CONTROLLER_STATE_Down)
    {
      int freeMem = freeMemory();
      char szTemp[16];
      sprintf(szTemp, "FREEMEM: %d", freeMem);
      debug.functln(szTemp);
    }

    // Check for modifiers to play back value or record value by holding down button
    uint32_t buttonTime;

    // Map moveTimeMinutes/movetimeHours to R1/R2
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_R1, &buttonTime, moveTimeMinutes ))
      moveTimeMinutes = (buttonTime / UI_TICK_RATE);
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_R2, &buttonTime, moveTimeHours ))
      moveTimeHours = (buttonTime / UI_TICK_RATE);

    // Map DECECL/ACCEL percentage to L1/L2
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_L1, &buttonTime, accelTime ))
      accelTime = (buttonTime / UI_TICK_RATE);
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_L2, &buttonTime, decelTime ))
      decelTime = (buttonTime / UI_TICK_RATE);

    // Map exposureTimeS/exposureTimeDS/exposureWaitS/exposureWaitDS to x/0/square/triangle
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_Cross, &buttonTime, exposureTimeS ))
      exposureTimeS = (buttonTime / UI_TICK_RATE);
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_Circle, &buttonTime, exposureTimeDS ))
      exposureTimeDS = (buttonTime / UI_TICK_RATE);
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_Square, &buttonTime, exposureWaitS ))
      exposureWaitS = (buttonTime / UI_TICK_RATE);
    if (MonitorButton( UI_MODIFIER_QUERY, PS3CONTROLLER_BUTTON_Triangle, &buttonTime, exposureWaitDS ))
      exposureWaitDS = (buttonTime / UI_TICK_RATE);

    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_PS) == PS3CONTROLLER_STATE_Down)
    {
      // Toggle SMS
      isContinuous = !isContinuous;
      PS3CtrlrHost.SetLED( 2, !isContinuous);
      }

    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Start) == PS3CONTROLLER_STATE_Down)
    {
      killTimerStart=millis();
      StartMove();
    }
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

/** Waiting state

  We wait for motors to finish running in this state

*/
void USBControllerUI::StateWait( void )
{
  uint8_t buttonState;

  if (isShotRunning == false )
  {
    state = USBCONTROLLERUI_STATE_Setting;
    return;
  }

  if ((millis() - shotStartTime) >= shotTimeMS)
  {
    isShotRunning = false;
    stopAllMotors();
    state = USBCONTROLLERUI_STATE_Setting;
    return;
  }

  buttonState = PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Start);
  if ( buttonState == PS3CONTROLLER_STATE_Down)
    killTimerStart = millis();

  if ( (buttonState == PS3CONTROLLER_STATE_On) && ((millis() - killTimerStart) > KILL_TIMER_TIME))
  {
    isShotRunning = false;
    stopAllMotors();
    state = USBCONTROLLERUI_STATE_Setting;
    return;
  }
}

/** Waiting to start

  Wait for motors to finish moving to start of move then
  start the move

*/
void USBControllerUI::StateWaitToStart( void )
{
  uint8_t buttonState;
  float fullSpeed;

  if (!areMotorsRunning())
  {
    // Start the move
    for (uint8_t i = 0; i < MOTOR_COUNT ; i++)
    {
      shotTimeMS = (60 * (uint32_t) moveTimeMinutes + 60 * 60 * (uint32_t) moveTimeHours) * 1000;

      motor[i].enable(true);
      motor[i].easing(OM_MOT_QUAD);

      if (isContinuous)
      {
        motor[i].planType( 1 );
        motor[i].continuous(true);
      }
      else
      {
        motor[i].planType( 0 );
        motor[i].continuous(false);
      }
      
      byte dir;
      if ((motor[i].stopPos() - motor[i].startPos()) > 0)
        dir = 1;
      else
        dir = 0;

      // Discard accel/decl if user input amount exceeding 100%
      uint32_t accelTimeMS;
      uint32_t decelTimeMS;
      if (decelTime + accelTime > 100)
      {
        accelTimeMS = 0;
        decelTimeMS = 0;
      }
      else
      {
        accelTimeMS = accelTime * (shotTimeMS / 100);
        decelTimeMS = decelTime * (shotTimeMS / 100);
      }

      fullSpeed = abs(motor[i].stopPos() - motor[i].startPos()) / ((60.0f * moveTimeMinutes + 60.0f * 60.0f * moveTimeHours));
      //
      // cruiseTimeMS = shotTimeMS - (accelTimeMS + decelTimeMS);
      //cruiseDistance = abs(motor[i].stopPos() - motor[i].startPos()) -

      motor[i].contSpeed( fullSpeed );
      motor[i].move(dir, abs(motor[i].stopPos() - motor[i].startPos()), shotTimeMS, accelTimeMS, decelTimeMS);
      
      // Set camera trigger parameters
      Camera.triggerTime((uint32_t) exposureTimeS * 1000 + (uint32_t) exposureTimeDS * 100);
      Camera.delayTime((uint32_t) exposureWaitS * 1000 + (uint32_t) exposureWaitDS * 100);
      Camera.focusTime(CAM_DEFAULT_FOCUS);
      Camera.setHandler(camCallBack);

      isShotRunning = true;
      shotStartTime = millis();
      startISR();
    }

    state = USBCONTROLLERUI_STATE_Wait;
    return;
  }

  buttonState = PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Start);
  if ( buttonState == PS3CONTROLLER_STATE_Down)
    killTimerStart = millis();

  if ( (buttonState == PS3CONTROLLER_STATE_On) && ((millis() - killTimerStart) > KILL_TIMER_TIME))
  {
    isShotRunning = false;
    stopAllMotors();
    state = USBCONTROLLERUI_STATE_Setting;
  }
}


/** Create Deadzone

  Create a deadzone so that the motors don't drift when sticks are idle

  value: input stick axis

  return: output stick axis with deadzone zero'd

*/
float USBControllerUI::CreateDeadzone( float value )
{
  if (abs(value - 0x80) <= UI_DEADZONE)
    return (0x80);
  else
    return (value);
}

/** Monitor button

  UI function for setting and querying various timers that are mapped
  to buttons on the USB controller.  If the user holds down a button
  and then let's go, it will return the number of seconds the button
  was held down while ticking an actuator every second.  If the user
  holds down the modifier button before pressing the button, the controller
  will vibrate queryValue number of times.

  modifierButton: button used to query the mapped button
  button: the mapped button
  storeMs: place to store the number of seconds when func returns true
  queryValue: number of ticks to vibrate when modifier is down

  return: true when user let's go of button without modifier
          false otherwise

*/
bool USBControllerUI::MonitorButton( PS3Controller_ButtonUsages_t modifierButton, PS3Controller_ButtonUsages_t button, uint32_t *storeMs, uint16_t queryValue )
{
  uint8_t buttonState;
  uint8_t modifierButtonState;
  static bool isQuery = false;
  static uint8_t isMonitoring = false;
  static uint8_t curButtonMonitoring = 0;
  static uint32_t buttonTimer;

  buttonState = PS3CtrlrHost.GetButtonState( button );
  modifierButtonState = PS3CtrlrHost.GetButtonState( modifierButton );

  // Only allow one button to be monitored
  if (isMonitoring == false || button == curButtonMonitoring)
  {
    if (buttonState == PS3CONTROLLER_STATE_Down && modifierButtonState == PS3CONTROLLER_STATE_Off)
    {
      curButtonMonitoring = button;
      isMonitoring = true;
      buttonTimer = millis();
      ActuatorPulse( 0, 255, UI_TICK_RATE * 0.15, UI_TICK_RATE * 0.85, 255);
    }
    else if (buttonState == PS3CONTROLLER_STATE_Down && ((modifierButtonState == PS3CONTROLLER_STATE_Down) || (modifierButtonState == PS3CONTROLLER_STATE_On)))
    {
      ActuatorPulse( 0, 255, UI_TICK_RATE * 0.15, UI_TICK_RATE * 0.85, queryValue);
      isQuery = true;
      return (false);
    }
    else if (buttonState == PS3CONTROLLER_STATE_Up && isQuery == true)
    {
      isQuery = false;
      return (false);
    }
    else if (buttonState == PS3CONTROLLER_STATE_Up && isQuery == false)
    {
      ActuatorPulseStop();
      isMonitoring = false;
      *storeMs = millis() - buttonTimer;
      return (true);
    }
  }
  return (false);
}

/** LED Pulse

  Pulse an LED.

  LEDNum: 1-4
  onMS: MS to keep LED on
  offMS: MS to keep LED off
  nTimes: Number of times to pulse

*/
void USBControllerUI::LEDPulse( uint8_t LEDNum, uint16_t onMS, uint16_t offMS, uint8_t nTimes)
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

/** LED Pulse Stop

  Stop pulsing this LED

  LEDNum: 1-4

*/
void USBControllerUI::LEDPulseStop( uint8_t LEDNum )
{
  //  LEDPulseState.isOn = false;
  PS3CtrlrHost.SetLED(LEDNum, false);
}

/** Actuator Pulse

  Pulse an actuator.

  ActNum: 0-1
  onMS: MS to keep actuator on
  offMS: MS to keep actuator off
  nTimes: Number of times to pulse

*/
void USBControllerUI::ActuatorPulse( uint8_t ActNum, uint8_t intensity, uint16_t onMS, uint16_t offMS, uint8_t nTimes)
{
  if (nTimes == 0)
    ActuatorPulseStop();
  else
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
}

/** Actuator Pulse Stop

  Stop pulsing actuators

*/
void USBControllerUI::ActuatorPulseStop( void )
{
  actuatorPulseState.isOn = false;
  if (actuatorPulseState.ActNum == 0)
    PS3CtrlrHost.SetSmallActuator(false, 0x00);

  if (actuatorPulseState.ActNum == 1)
    PS3CtrlrHost.SetBigActuator(0, 0x00);
}

/** Iterate Pulses

  Tick pulsing states one frame forward

*/
void USBControllerUI::IteratePulses( void )
{
  uint32_t timeElapsed;
  uint8_t i;

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

/** Start Move

  Move all motors to start position and start motor move states.

*/
void USBControllerUI::StartMove( void )
{
  // Go to start of move
  sendAllToStart();

  state = USBCONTROLLERUI_STATE_WaitToStart;
}




