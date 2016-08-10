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
#define FAST_UI_TICK_RATE     (500)
#define SLOW_UI_TICK_RATE     (1000)
#define RUNNING_LED_TICK_RATE (FAST_UI_TICK_RATE)
#define BUTTON_TIMER_TIME (2000)
#define UI_LOADSAVE_TIMER_TIME (1000)

#define USBCTRLR_MAX_MOTORS (3)

#define UI_LED_CONNECTED    (1)
#define UI_LED_SMS          (2)
#define UI_LED_CAMERA     (3)
#define UI_LED_RUNNING      (4)

#define UI_BUTTON_ShotTimeHours    PS3CONTROLLER_BUTTON_R1
#define UI_BUTTON_ShotTimeMinutes  PS3CONTROLLER_BUTTON_R2
#define UI_BUTTON_AccelPercentage  PS3CONTROLLER_BUTTON_L1
#define UI_BUTTON_DecelPercentage  PS3CONTROLLER_BUTTON_L2
#define UI_BUTTON_IntervalTimeS    PS3CONTROLLER_BUTTON_Cross
#define UI_BUTTON_IntervalTimeDS   PS3CONTROLLER_BUTTON_Circle
#define UI_BUTTON_ExposureTimeS    PS3CONTROLLER_BUTTON_Square
#define UI_BUTTON_ExposureTimeDS   PS3CONTROLLER_BUTTON_Triangle
#define UI_BUTTON_FocusTimeDS      PS3CONTROLLER_BUTTON_Up
#define UI_BUTTON_ExposureWaitDS   PS3CONTROLLER_BUTTON_Down
#define UI_BUTTON_SetStart         PS3CONTROLLER_BUTTON_Left
#define UI_BUTTON_SetStop          PS3CONTROLLER_BUTTON_Right
#define UI_BUTTON_ToggleMode       PS3CONTROLLER_BUTTON_PS
#define UI_BUTTON_StartShot        PS3CONTROLLER_BUTTON_Start
#define UI_BUTTON_StopShot         PS3CONTROLLER_BUTTON_Start
#define UI_BUTTON_Modifier         PS3CONTROLLER_BUTTON_Select
#define UI_BUTTON_SaveSetting      PS3CONTROLLER_BUTTON_R3
#define UI_BUTTON_LoadSetting      PS3CONTROLLER_BUTTON_L3


const uint32_t thousand = 1000;
const uint32_t onehundred = 100;


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
  prevLeftXVelocity = 128;
  prevLeftYVelocity = 128;
  prevRightXVelocity = 128;
  prevRightYVelocity = 128;

  actuatorPulseState.isOn = false;
  
  ResetUIDefaults();
  
  isShotRunning = false;

  for (uint8_t i = 0 ; i < USBCTRLR_MAX_MOTORS ; i++)
  {
    motor[i].easing(OM_MOT_QUAD);
    if (i == 0)
      motor[i].ms(4);
    else
      motor[i].ms(16);
  }

  Camera.exposureFocus(false);
  Camera.enable = true;

  uiState = USBCONTROLLERUI_STATE_Setting;
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

  if (PS3CtrlrHost.IsConnected() == true && prevStatus == false)
  {
    PS3CtrlrHost.SetLED( UI_LED_CONNECTED, true );
    PS3CtrlrHost.SetLED( UI_LED_SMS, !uiSettings.isContinuous); 
  }

  prevStatus = PS3CtrlrHost.IsConnected();

  // Run USB Controller State machine
  if (uiState == USBCONTROLLERUI_STATE_Setting)
  {
    uiStateSetting();
  }
  else if (uiState == USBCONTROLLERUI_STATE_Wait)
  {
    uiStateWait();
  }
  else if (uiState == USBCONTROLLERUI_STATE_WaitToStart)
  {
    uiStateWaitToStart();
  }
  IteratePulses();	 
}

/** Setting State

  In this UI state, information regarding the move can be input including
  start/stop points and timers.

*/
void USBControllerUI::uiStateSetting( void )
{
  if (PS3CtrlrHost.IsConnected())
  {
    // Stop drifting when idle
    float leftXVelocity = CreateDeadzone(PS3CtrlrHost.LeftStickX);
    float leftYVelocity = CreateDeadzone(PS3CtrlrHost.LeftStickY);
    float rightXVelocity = CreateDeadzone(PS3CtrlrHost.RightStickX);
    float rightYVelocity = CreateDeadzone(PS3CtrlrHost.RightStickY);

    // Set Microstep 
    motor[0].ms(4);
    motor[1].ms(16);
    motor[2].ms(16);
    
    // Map PS3 sticks to first four axis
    if (leftXVelocity != prevLeftXVelocity) setJoystickSpeed(0, ((leftXVelocity - 128.0f) / 128.0f)* mot_max_speed );
    if (rightXVelocity != prevRightXVelocity) setJoystickSpeed(1, -((rightXVelocity - 128.0f) / 128.0f)* mot_max_speed );
    if (rightYVelocity != prevRightYVelocity) setJoystickSpeed(2, -((rightYVelocity - 128.0f) / 128.0f)* mot_max_speed );

    prevLeftXVelocity =  leftXVelocity;
    prevLeftYVelocity = leftYVelocity;
    prevRightXVelocity = rightXVelocity;
    prevRightYVelocity = rightYVelocity;

    uint8_t modifierButtonState;
    uint8_t buttonState;

    modifierButtonState = PS3CtrlrHost.GetButtonState( UI_BUTTON_Modifier );

    // Map DPAD Left/Right to Start/Stop points
    if (PS3CtrlrHost.GetButtonState( UI_BUTTON_SetStart ) == PS3CONTROLLER_STATE_Down)
    {
      if (modifierButtonState == PS3CONTROLLER_STATE_Down || modifierButtonState == PS3CONTROLLER_STATE_On)
      {
        sendAllToStart();
      }
      else
      {
        ConfirmPulse();
        motor[0].startPos(motor[0].currentPos());  // Set start point on axis 0
        motor[1].startPos(motor[1].currentPos());  // Set start point on axis 1
        motor[2].startPos(motor[2].currentPos());  // Set start point on axis 2
      }
    }
    if (PS3CtrlrHost.GetButtonState( UI_BUTTON_SetStop ) == PS3CONTROLLER_STATE_Down)
    { if (modifierButtonState == PS3CONTROLLER_STATE_Down || modifierButtonState == PS3CONTROLLER_STATE_On)
      {
        sendAllToStop();
      }
      else
      {
        ConfirmPulse();
        motor[0].stopPos(motor[0].currentPos());  // Set stop point on axis 0
        motor[1].stopPos(motor[1].currentPos());  // Set stop point on axis 1
        motor[2].stopPos(motor[2].currentPos());  // Set stop point on axis 2
      }
    }

    // Check for modifiers to play back value or record value by holding down button
    uint8_t monitorValue;
    
    // Map moveTimeMinutes/movetimeHours to R1/R2
    if (MonitorButton( modifierButtonState, UI_BUTTON_ShotTimeHours, &monitorValue, uiSettings.moveTimeMinutes, FAST_UI_TICK_RATE ))
      uiSettings.moveTimeMinutes = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_ShotTimeMinutes, &monitorValue, uiSettings.moveTimeHours, FAST_UI_TICK_RATE ))
      uiSettings.moveTimeHours = monitorValue;

    // Map DECECL/ACCEL percentage to L1/L2
    if (MonitorButton( modifierButtonState, UI_BUTTON_AccelPercentage, &monitorValue, uiSettings.accelPercentage, FAST_UI_TICK_RATE ))
      uiSettings.accelPercentage = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_DecelPercentage, &monitorValue, uiSettings.decelPercentage, FAST_UI_TICK_RATE ))
      uiSettings.decelPercentage = monitorValue;

    // Map timers/delays
    if (MonitorButton( modifierButtonState, UI_BUTTON_IntervalTimeS, &monitorValue, uiSettings.intervalTimeS, FAST_UI_TICK_RATE ))
      uiSettings.intervalTimeS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_IntervalTimeDS, &monitorValue, uiSettings.intervalTimeDS, FAST_UI_TICK_RATE ))
      uiSettings.intervalTimeDS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_ExposureTimeS, &monitorValue, uiSettings.exposureTimeS, FAST_UI_TICK_RATE ))
      uiSettings.exposureTimeS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_ExposureTimeDS, &monitorValue, uiSettings.exposureTimeDS, FAST_UI_TICK_RATE ))
      uiSettings.exposureTimeDS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_FocusTimeDS, &monitorValue, uiSettings.focusTimeDS, FAST_UI_TICK_RATE ))
      uiSettings.focusTimeDS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_ExposureWaitDS, &monitorValue, uiSettings.exposureWaitDS, FAST_UI_TICK_RATE ))
      uiSettings.exposureWaitDS = monitorValue;
    
    // Load/Save Settings UI
    if (MonitorButton( modifierButtonState, UI_BUTTON_SaveSetting, &monitorValue, 0, SLOW_UI_TICK_RATE ))
    {  
      if(monitorValue > 0 && monitorValue < (USBCONTROLLERUI_NSETTINGS+1))
      {
        SaveUISetting( monitorValue-1 );
      }
    }
    
    if (MonitorButton( modifierButtonState, UI_BUTTON_LoadSetting, &monitorValue, 0, SLOW_UI_TICK_RATE ))
    {  
      if(monitorValue > 0 && monitorValue < (USBCONTROLLERUI_NSETTINGS+1))
      {  
        LoadUISetting( monitorValue-1 );
      }
    }
    
    if (PS3CtrlrHost.GetButtonState(UI_BUTTON_ToggleMode) == PS3CONTROLLER_STATE_Down)
    {
      // Toggle SMS
      uiSettings.isContinuous = !uiSettings.isContinuous;
      PS3CtrlrHost.SetLED( UI_LED_SMS, !uiSettings.isContinuous);
    }

    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Start) == PS3CONTROLLER_STATE_Down)
    {
      if(PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Select) == PS3CONTROLLER_STATE_Down || PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Select) == PS3CONTROLLER_STATE_On)
      {
        // Reset Setting Default
        ResetUIDefaults();
        SaveUISetting( 0 );
        ConfirmPulse();
      }
      else
      {
        // Start Shot
        buttonTimerStart = millis();
        TriggerStartMove();
      }
    }
  }
  else
  {
    if (prevLeftXVelocity != 128) {
      prevLeftXVelocity = 128;
      setJoystickSpeed(0, 0 );
    }
    if (prevLeftYVelocity != 128) {
      prevLeftYVelocity = 128;
      setJoystickSpeed(2, 0 );
    }
    if (prevRightXVelocity != 128) {
      prevRightXVelocity = 128;
      setJoystickSpeed(1, 0 );
    }
    if (prevRightYVelocity != 128) {
      setJoystickSpeed(3, 0 );
      prevRightYVelocity = 128;
    }
  }
}

/** Waiting state

  We wait for motors to finish running in this state

*/
void USBControllerUI::uiStateWait( void )
{
  uint8_t buttonState;
  static uint32_t runningLEDTimer=millis();
  static uint8_t runningLEDState=false, cameraLEDState;

  if (isShotRunning == false || !running )
  {
    runningLEDState = false;
    cameraLEDState = false;
    PS3CtrlrHost.SetLED( UI_LED_RUNNING, runningLEDState );
    PS3CtrlrHost.SetLED( UI_LED_CAMERA, cameraLEDState);
    uiState = USBCONTROLLERUI_STATE_Setting;
    return;
  }

  // Check if user is stopping the shot
  buttonState = PS3CtrlrHost.GetButtonState(UI_BUTTON_StopShot);
  if ( buttonState == PS3CONTROLLER_STATE_Down)
    buttonTimerStart = millis();
  if ( (buttonState == PS3CONTROLLER_STATE_On) && ((millis() - buttonTimerStart) > BUTTON_TIMER_TIME))
  {
    runningLEDState = false;
    cameraLEDState = false;
    PS3CtrlrHost.SetLED( UI_LED_RUNNING, runningLEDState );
    PS3CtrlrHost.SetLED( UI_LED_CAMERA, cameraLEDState);
    uiState = USBCONTROLLERUI_STATE_Setting;
    StopMove();
    return;
  }
  
  // Allow user to query the time left using R1/R2
  uint32_t shotTimeLeftMS = shotTimeMS-(millis()- shotStartTime);
  uint8_t timeLeftHours = ((shotTimeLeftMS)/thousand)/(60*60);
  uint8_t timeLeftMinutes = ((shotTimeLeftMS)/(60*thousand))-(timeLeftHours*60);
  
  // Allow settings to be queried while shot is running
  QueryButton( UI_BUTTON_ShotTimeHours, timeLeftMinutes, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_ShotTimeMinutes, timeLeftHours, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_AccelPercentage, uiSettings.accelPercentage, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_DecelPercentage, uiSettings.decelPercentage, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_IntervalTimeS, uiSettings.intervalTimeS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_IntervalTimeDS, uiSettings.intervalTimeDS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_ExposureTimeS, uiSettings.exposureTimeS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_ExposureTimeDS, uiSettings.exposureTimeDS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_FocusTimeDS , uiSettings.focusTimeDS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_ExposureWaitDS, uiSettings.exposureWaitDS, FAST_UI_TICK_RATE);
  
  // Flash RUNNING LED
  if((millis()-runningLEDTimer) >= RUNNING_LED_TICK_RATE)
  {
    runningLEDState = !runningLEDState;
    PS3CtrlrHost.SetLED( UI_LED_RUNNING, runningLEDState );
    runningLEDTimer = millis();
  }
  
  // Flash BUSY LED
  if(Camera.busy())
  {
    if(!cameraLEDState)
    {
      cameraLEDState = true;
      PS3CtrlrHost.SetLED( UI_LED_CAMERA, cameraLEDState);     
    }
  }
  else
  {
    if(cameraLEDState)
    {
      cameraLEDState = false;
      PS3CtrlrHost.SetLED( UI_LED_CAMERA, cameraLEDState); 
    }
  }
  
  // Turn on LED while camera is exposing
  return;
}

/** Stop Move

  Stop the shot/move

*/
void USBControllerUI::StopMove( void )
{
  stopProgram(true);

  isShotRunning = false;

  // Restore full speed microstep setting
  for (uint8_t i = 0 ; i < USBCTRLR_MAX_MOTORS ; i++)
  {
    if (i == 0)
      motor[i].ms(4);
    else
      motor[i].ms(16);
  }
}

/** Start Move

  Set motor and camera parameters and begin the shot/move.

*/
void USBControllerUI::StartMove( void )
{
  uint32_t exposureTime = uiSettings.exposureTimeS * thousand + uiSettings.exposureTimeDS * onehundred;
  uint32_t focusTime = uiSettings.focusTimeS * thousand + uiSettings.focusTimeDS * onehundred;
  uint32_t exposureDelay = uiSettings.exposureWaitS * thousand + uiSettings.exposureWaitDS * onehundred;
  uint32_t intervalTimeMS = uiSettings.intervalTimeS*thousand+uiSettings.intervalTimeDS*onehundred;
  
  float accelFraction = uiSettings.accelPercentage/100.0f;
  float decelFraction = uiSettings.accelPercentage/100.0f;
  
  shotTimeMS = (60 * (uint32_t) uiSettings.moveTimeMinutes + 60 * 60 * (uint32_t) uiSettings.moveTimeHours);
  shotTimeMS *= thousand;

  Motors::planType(uiSettings.isContinuous);

  pingPongMode(false);

  Camera.intervalTime(intervalTimeMS);
  Camera.focusTime( focusTime );
  Camera.triggerTime(exposureTime);
  Camera.delayTime(exposureDelay);

  // Assign motor parameters
  for (uint8_t i = 0; i < USBCTRLR_MAX_MOTORS; i++) {
    motor[i].clear();
    motor[i].planLeadIn(0);
    motor[i].planLeadOut(0);
    motor[i].easing(OM_MOT_QUAD);
    motor[i].planType( uiSettings.isContinuous );
    motor[i].planRun();
    if (!uiSettings.isContinuous)
    {
      uint32_t travelLength = shotTimeMS / (uiSettings.intervalTimeS*thousand + uiSettings.intervalTimeDS*onehundred );    
      
      // Number of Shots
      motor[i].planAccelLength(accelFraction*travelLength);
      motor[i].planDecelLength(decelFraction*travelLength);
      
      motor[i].planTravelLength( travelLength ) ;
      // motor[i].planRun();
      cameraAutoMaxShots();
    }
    else
    {
      // Length of time in MS
      motor[i].planAccelLength( accelFraction * shotTimeMS );
      motor[i].planDecelLength( decelFraction * shotTimeMS );
      motor[i].planTravelLength( shotTimeMS );
    }
  }
  shotStartTime = millis();
  
  isShotRunning = true;
  startProgramCom();
}


/** Waiting to start

  Wait for motors to finish moving to start of move then
  start the move

*/
void USBControllerUI::uiStateWaitToStart( void )
{
  uint8_t buttonState = PS3CtrlrHost.GetButtonState(UI_BUTTON_StopShot);
  
  if (!areMotorsRunning() && buttonState == PS3CONTROLLER_STATE_Up)
  {
    StartMove();

    uiState = USBCONTROLLERUI_STATE_Wait;
    return;
  }


  if ( buttonState == PS3CONTROLLER_STATE_Down)
    buttonTimerStart = millis();

  if ( (buttonState == PS3CONTROLLER_STATE_On) && ((millis() - buttonTimerStart) > BUTTON_TIMER_TIME))
  {
    isShotRunning = false;
    stopAllMotors();
    uiState = USBCONTROLLERUI_STATE_Setting;
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

/** Monitor button Query Only

  The controller will vibrate queryValue number of times if the selected 
  button is pressed.

  button: the mapped button
  queryValue: number of ticks to vibrate when modifier is down
  tickRate: number of milliseconds per tick

*/
void USBControllerUI::QueryButton( PS3Controller_ButtonUsages_t button, uint16_t queryValue, uint16_t tickRate )
{
  uint8_t buttonState = PS3CtrlrHost.GetButtonState( button );
  if(buttonState == PS3CONTROLLER_STATE_Down)
    ActuatorPulse( tickRate * 0.15, tickRate * 0.85, queryValue);
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
  tickRate: number of milliseconds per tick

  return: true when user let's go of button without modifier
          false otherwise

*/
uint8_t USBControllerUI::MonitorButton( uint8_t modifierButtonState, PS3Controller_ButtonUsages_t button, uint8_t *storeValue, uint16_t queryValue, uint16_t tickRate )
{
  uint8_t buttonState;
  static uint8_t isQuery = false;
  static uint8_t isMonitoring = false;
  static uint8_t curButtonMonitoring = 0;
  static uint32_t buttonTimer;

  buttonState = PS3CtrlrHost.GetButtonState( button );

  // Only allow one button to be monitored
  if (isMonitoring == false || button == curButtonMonitoring)
  {
    if (buttonState == PS3CONTROLLER_STATE_Down && modifierButtonState == PS3CONTROLLER_STATE_Off)
    {
      curButtonMonitoring = button;
      isMonitoring = true;
      buttonTimer = millis();
      ActuatorPulse( tickRate * 0.15, tickRate * 0.85, 255);
    }
    else if (buttonState == PS3CONTROLLER_STATE_Down && ((modifierButtonState == PS3CONTROLLER_STATE_Down) || (modifierButtonState == PS3CONTROLLER_STATE_On)))
    {
      ActuatorPulse( tickRate * 0.15, tickRate * 0.85, queryValue);
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
      *storeValue = actuatorPulseState.nPulses;
      return (true);
    }
  }
  return (false);
}

/** Actuator Pulse

  Pulse an actuator.

  ActNum: 0-1
  onMS: MS to keep actuator on
  offMS: MS to keep actuator off
  nTimes: Number of times to pulse

*/
void USBControllerUI::ActuatorPulse( uint16_t onMS, uint16_t offMS, uint8_t nTimes)
{
  if (nTimes == 0)
    ActuatorPulseStop();
  else
  {
    actuatorPulseState.isOn = true;
    actuatorPulseState.pulsesLeft = nTimes;
    actuatorPulseState.nPulses = 0;
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
  PS3CtrlrHost.SetSmallActuator(false, 0x00);
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
        actuatorPulseState.nPulses++;
        PS3CtrlrHost.SetSmallActuator( true, 0xFE);
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
        PS3CtrlrHost.SetSmallActuator( false, 0xFE);
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
void USBControllerUI::TriggerStartMove( void )
{
  // Go to start of move1
  sendAllToStart();

  uiState = USBCONTROLLERUI_STATE_WaitToStart;
}


/** Reset UI Defaults

  Reset setting zero/default to the default parameters for a shot.

*/
void USBControllerUI::ResetUIDefaults( void )
{
  uiSettings.moveTimeMinutes = 5;
  uiSettings.moveTimeHours = 0;
  uiSettings.accelPercentage = 0;
  uiSettings.decelPercentage = 0;
  uiSettings.isContinuous = true;
  uiSettings.exposureTimeS = 0;
  uiSettings.exposureWaitS = 0;
  uiSettings.exposureTimeDS = 1;
  uiSettings.exposureWaitDS = 2;
  uiSettings.focusTimeS = 0;
  uiSettings.focusTimeDS = 0;
  uiSettings.intervalTimeS = 1;
  uiSettings.intervalTimeDS = 0;
  
  PS3CtrlrHost.SetLED( UI_LED_SMS, !uiSettings.isContinuous );
}

/** Save a UI Setting

  Load a setting into the current shots setting.  nSetting must be less than
  USBCONTROLLERUI_NSETTINGS.
  
*/
void USBControllerUI::SaveUISetting( uint8_t nSetting )
{
  char *tempPtr = (char *) &USBCtrlrUI.uiSettings;
  
  for(int j=0;j<sizeof(CtrlrUISettings_t);j++)
  {
    OMEEPROM::write( EE_USBCTRLR_SETTINGS + sizeof(CtrlrUISettings_t)*nSetting+ j, tempPtr[j]);
  }
}

/** Load UI Setting

  Load a setting into the current shots setting.  nSetting must be less than
  USBCONTROLLERUI_NSETTINGS.
  
*/
void USBControllerUI::LoadUISetting( uint8_t nSetting )
{
  char *tempPtr = (char *) &USBCtrlrUI.uiSettings;
  for(int j=0;j<sizeof(CtrlrUISettings_t);j++)
  {
    OMEEPROM::read( EE_USBCTRLR_SETTINGS + sizeof(CtrlrUISettings_t)*nSetting + j, tempPtr[j]);
  }
  
  // Set controller UI stuff associated with this setting
  PS3CtrlrHost.SetLED( UI_LED_SMS, !uiSettings.isContinuous); 
}

/** Confirm Pulse

  Pulse Actuator to confirm action
  
*/
void USBControllerUI::ConfirmPulse( void )
{
  ActuatorPulse( 150, 100, 1);
}

/** Is Shot Running?

  Query whether a USB Controller shot is running
  
*/
uint8_t USBControllerUI::IsShotRunning( void )
{
  return(isShotRunning);
}

