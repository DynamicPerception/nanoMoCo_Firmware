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
#define UI_REFRESH_TIME (10)  

#define UI_LED_CONNECTED    (1)
#define UI_LED_SMS          (2)
#define UI_LED_CAMERA     (3)
#define UI_LED_RUNNING      (4)
#define UI_INPUTFILTER_COUNT (3)

#define UI_BUTTON_ShotTimeHours    PS3CONTROLLER_BUTTON_R1
#define UI_BUTTON_ShotTimeMinutes  PS3CONTROLLER_BUTTON_R2
#define UI_BUTTON_AccelPercentage  PS3CONTROLLER_BUTTON_L1
#define UI_BUTTON_DecelPercentage  PS3CONTROLLER_BUTTON_L2
#define UI_BUTTON_IntervalTimeS    PS3CONTROLLER_BUTTON_Cross
#define UI_BUTTON_ExposureTimeS    PS3CONTROLLER_BUTTON_Square
#define UI_BUTTON_ExposureTimeDS   PS3CONTROLLER_BUTTON_Triangle
#define UI_BUTTON_SetStart         PS3CONTROLLER_BUTTON_Left
#define UI_BUTTON_SetStop          PS3CONTROLLER_BUTTON_Right
#define UI_BUTTON_ToggleMode       PS3CONTROLLER_BUTTON_PS
#define UI_BUTTON_StartShot        PS3CONTROLLER_BUTTON_Start
#define UI_BUTTON_StopShot         PS3CONTROLLER_BUTTON_Start
#define UI_BUTTON_Modifier         PS3CONTROLLER_BUTTON_Select
#define UI_BUTTON_SaveSetting      PS3CONTROLLER_BUTTON_Down
#define UI_BUTTON_LoadSetting      PS3CONTROLLER_BUTTON_Up
#define UI_BUTTON_FocusTimeDS      PS3CONTROLLER_BUTTON_Circle
#define UI_BUTTON_Leadin           PS3CONTROLLER_BUTTON_Left
#define UI_BUTTON_Leadout          PS3CONTROLLER_BUTTON_Right
#define UI_BUTTON_AxisActivation   PS3CONTROLLER_BUTTON_R3

enum{
  UI_MOTOR_ACTSTATE_PANANDTILT=0,
  UI_MOTOR_ACTSTATE_PAN,
  UI_MOTOR_ACTSTATE_TILT
};


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
  prevRightXVelocity = 128;
  prevRightYVelocity = 128;
  readyToStart = false;
  isJoystickOwner = false;
  motorActivationStatus = UI_MOTOR_ACTSTATE_PANANDTILT;
  
  actuatorPulseState.isOn = false;

  ResetUIDefaults();

  isShotRunning = false;

  Camera.exposureFocus(false);
  Camera.enable = true;

  uiState = USBCONTROLLERUI_STATE_Setting;
}

/** UI Task

  Perform per frame tasks for UI

*/
void USBControllerUI::UITask( void )
{
  static uint32_t prevTime;

  if ((millis() - prevTime) < UI_REFRESH_TIME)
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
    leftXFilterCount=0;
    rightXFilterCount=0;
    rightYFilterCount=0;
    prevLeftXVelocity = 128;
    prevRightXVelocity = 128;
    prevRightYVelocity = 128;
  }

  prevStatus = PS3CtrlrHost.IsConnected();

  // Run USB Controller State machine
  if (uiState == USBCONTROLLERUI_STATE_Setting) {
    uiStateSetting();
  }
  else if (uiState == USBCONTROLLERUI_STATE_Wait) {
    uiStateWait();
  }
  else if (uiState == USBCONTROLLERUI_STATE_WaitToSetting) {
    uiStateWaitToSetting();
  }
  else if (uiState == USBCONTROLLERUI_STATE_WaitToStop) {
    uiStateWaitToStop();
  }
  IteratePulses();
}



/** Setting State

  In this UI state, information regarding the move can be input including
  start/stop points and timers.

*/
void USBControllerUI::uiStateSetting( void )
{
  // If another app is running joystick mode, ignore PS3 Controller
  if ((isJoystickOwner == false && joystickSet() == true) || (getRunStatus() > 0))
    return;

  if (PS3CtrlrHost.IsConnected() == true)
  {
    // Stop drifting when idle
    float leftXVelocity = CreateDeadzone(PS3CtrlrHost.GetLeftStickX());
    float rightXVelocity = CreateDeadzone(PS3CtrlrHost.GetRightStickX());
    float rightYVelocity = CreateDeadzone(PS3CtrlrHost.GetRightStickY());
  
    // Ignore recoil bumps on the sticks
    if((leftXVelocity > 128 && prevLeftXVelocity < 128) || (leftXVelocity < 128 && prevLeftXVelocity > 128)) 
      leftXFilterCount=UI_INPUTFILTER_COUNT;  
    if(leftXFilterCount > 0) {
      leftXVelocity = 128;
       leftXFilterCount--;
    }
    if((rightXVelocity > 128 && prevRightXVelocity < 128) || (rightXVelocity < 128 && prevRightXVelocity > 128)) 
      rightXFilterCount=UI_INPUTFILTER_COUNT;  
    if(rightXFilterCount > 0) {
      rightXVelocity = 128;
       rightXFilterCount--;
    }
    if((rightYVelocity > 128 && prevRightYVelocity < 128) || (rightYVelocity < 128 && prevRightYVelocity > 128)) 
      rightYFilterCount=UI_INPUTFILTER_COUNT;  
    if(rightYFilterCount > 0) {
      rightYVelocity = 128;
       rightYFilterCount--;
    }

    if ((leftXVelocity != 128 ||
        rightXVelocity != 128 ||
        rightYVelocity != 128) && isJoystickOwner == false)
    {
      isJoystickOwner = true;
      joystickSet(true);
    }

    // Map PS3 sticks to first three axis
    if(leftXVelocity != prevLeftXVelocity)
    {
      readyToStart = false;
      motor[0].ms(microstepSettings[0]);
      setJoystickSpeed(0, ((leftXVelocity - 128.0f) / 128.0f)* motor[0].maxSpeed() );
    }
    if ((rightXVelocity != prevRightXVelocity) && ( motorActivationStatus == UI_MOTOR_ACTSTATE_PAN || motorActivationStatus == UI_MOTOR_ACTSTATE_PANANDTILT )) {
      readyToStart = false;
      motor[1].ms(microstepSettings[1]);
      setJoystickSpeed(1, -((rightXVelocity - 128.0f) / 128.0f)* motor[1].maxSpeed() );
    }
    
    if ((rightYVelocity != prevRightYVelocity) && ( motorActivationStatus == UI_MOTOR_ACTSTATE_TILT || motorActivationStatus == UI_MOTOR_ACTSTATE_PANANDTILT )) {
      readyToStart = false;
      motor[2].ms(microstepSettings[2]);
      setJoystickSpeed(2, -((rightYVelocity - 128.0f) / 128.0f)* motor[2].maxSpeed() );
    }

    prevLeftXVelocity =  leftXVelocity;
    prevRightXVelocity = rightXVelocity;
    prevRightYVelocity = rightYVelocity;

    uint8_t modifierButtonState;
    uint8_t buttonState;
    uint8_t monitorValue;

    modifierButtonState = PS3CtrlrHost.GetButtonState( UI_BUTTON_Modifier );
    if(PS3CtrlrHost.PeekButtonState( UI_BUTTON_Modifier ) && (PS3CtrlrHost.PeekButtonState( UI_BUTTON_SetStart) == PS3CONTROLLER_STATE_Down))
    {
        uiState = USBCONTROLLERUI_STATE_WaitToSetting;
        sendAllToStart();
        readyToStart = true;
        return;  
    }
    
    if (MonitorButton( modifierButtonState, UI_BUTTON_SetStart, &monitorValue, 0, FAST_UI_TICK_RATE, 255 ))
    { 
      if(monitorValue == 0)
      {
        ActuatorPulse( 400, 10, 1);
        for (uint8_t i = 0; i < USBCONTROLLERUI_NMOTORS ; i++)
          motor[i].startPos(motor[i].currentPos());  // Set start point on axis
      }  
      else
      {  
        uiSettings.leadinMinutes = monitorValue-1;
      }  
  } 
    
    if(PS3CtrlrHost.PeekButtonState( UI_BUTTON_Modifier ) && (PS3CtrlrHost.PeekButtonState( UI_BUTTON_SetStop) == PS3CONTROLLER_STATE_Down))
    {
        uiState = USBCONTROLLERUI_STATE_WaitToSetting;
        sendAllToStop();
        return;  
    }
    
    if (MonitorButton( modifierButtonState, UI_BUTTON_SetStop, &monitorValue, 0, FAST_UI_TICK_RATE, 255 ))
    { 
      if(monitorValue == 0)
      {
        ActuatorPulse( 400, 10, 1);
        for (uint8_t i = 0; i < USBCONTROLLERUI_NMOTORS ; i++)
          motor[i].stopPos(motor[i].currentPos());  // Set start point on axis
      }  
      else
        uiSettings.leadoutMinutes = monitorValue-1;
    } 
 
    // Map R3 to motor activation state
    if (MonitorButton( modifierButtonState,  UI_BUTTON_AxisActivation, &monitorValue, motorActivationStatus, FAST_UI_TICK_RATE, 1 ))
    {  
      if(monitorValue == 1)
      {   
        // Toggle Motor activation
        switch(motorActivationStatus)
        {
          case UI_MOTOR_ACTSTATE_PANANDTILT:
            motorActivationStatus = UI_MOTOR_ACTSTATE_PAN;
            break;
          case UI_MOTOR_ACTSTATE_PAN:
            motorActivationStatus = UI_MOTOR_ACTSTATE_TILT;
            break;
          case UI_MOTOR_ACTSTATE_TILT:
            motorActivationStatus = UI_MOTOR_ACTSTATE_PANANDTILT;
            break;
        }
      }
    } 

    // Map moveTimeMinutes/movetimeHours to R1/R2
    if (MonitorButton( modifierButtonState, UI_BUTTON_ShotTimeHours, &monitorValue, uiSettings.moveTimeMinutes, FAST_UI_TICK_RATE, 255 ))
      uiSettings.moveTimeMinutes = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_ShotTimeMinutes, &monitorValue, uiSettings.moveTimeHours, FAST_UI_TICK_RATE, 255 ))
      uiSettings.moveTimeHours = monitorValue;

    // Map DECECL/ACCEL percentage to L1/L2
    if (MonitorButton( modifierButtonState, UI_BUTTON_AccelPercentage, &monitorValue, uiSettings.accelPercentage, FAST_UI_TICK_RATE, 100 ))
      uiSettings.accelPercentage = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_DecelPercentage, &monitorValue, uiSettings.decelPercentage, FAST_UI_TICK_RATE, 100 ))
      uiSettings.decelPercentage = monitorValue;

    // Map timers/delays
    if (MonitorButton( modifierButtonState, UI_BUTTON_IntervalTimeS, &monitorValue, uiSettings.intervalTimeS, FAST_UI_TICK_RATE, 255 ))
      uiSettings.intervalTimeS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_FocusTimeDS, &monitorValue, uiSettings.focusTimeDS, FAST_UI_TICK_RATE, 10 ))
      uiSettings.focusTimeDS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_ExposureTimeS, &monitorValue, uiSettings.exposureTimeS, FAST_UI_TICK_RATE, 255 ))
      uiSettings.exposureTimeS = monitorValue;
    if (MonitorButton( modifierButtonState, UI_BUTTON_ExposureTimeDS, &monitorValue, uiSettings.exposureTimeDS, FAST_UI_TICK_RATE, 10 ))
      uiSettings.exposureTimeDS = monitorValue;


    // Load/Save Settings UI
    if (MonitorButton( modifierButtonState, UI_BUTTON_SaveSetting, &monitorValue, 0, SLOW_UI_TICK_RATE, 3 ))
    {
      if (monitorValue > 0 && monitorValue < (USBCONTROLLERUI_NSETTINGS + 1))
      {
        SaveUISetting( monitorValue - 1 );
      }
    }

    if (MonitorButton( modifierButtonState, UI_BUTTON_LoadSetting, &monitorValue, 0, SLOW_UI_TICK_RATE, 3 ))
    {
      if (monitorValue > 0 && monitorValue < (USBCONTROLLERUI_NSETTINGS + 1))
      {
        LoadUISetting( monitorValue - 1 );
      }
    }

    if (PS3CtrlrHost.GetButtonState(UI_BUTTON_ToggleMode) == PS3CONTROLLER_STATE_Down)
    {
      // Toggle SMS
      uiSettings.isContinuous = !uiSettings.isContinuous;
      PS3CtrlrHost.SetLED( UI_LED_SMS, !uiSettings.isContinuous);
    }

    if (PS3CtrlrHost.GetButtonState(UI_BUTTON_ToggleMode) == PS3CONTROLLER_STATE_Down)
    {
      // Toggle motor activation
      uiSettings.isContinuous = !uiSettings.isContinuous;
      PS3CtrlrHost.SetLED( UI_LED_SMS, !uiSettings.isContinuous);
    }
    
    if ((motor[0].currentPos() != motor[0].startPos()) ||
        (motor[1].currentPos() != motor[1].startPos()) ||
        (motor[2].currentPos() != motor[2].startPos()))
    {
      readyToStart = false;
    }

    if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Start) == PS3CONTROLLER_STATE_Down)
    {
      if (PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Select) == PS3CONTROLLER_STATE_Down || PS3CtrlrHost.GetButtonState(PS3CONTROLLER_BUTTON_Select) == PS3CONTROLLER_STATE_On)
      {
        // Reset Setting Default
        ResetUIDefaults();
        SaveUISetting( 0 );
        ConfirmPulse();
      }
      else if (readyToStart == true)
      {
        // Start Shot
        readyToStart = false;
        buttonTimerStart = millis();
        uiState = USBCONTROLLERUI_STATE_Wait;
        
        StartMove();
        return;
      }
      else if (readyToStart == false)
      {
        // Goto start of move
        readyToStart = true;
        buttonTimerStart = millis();
        uiState = USBCONTROLLERUI_STATE_WaitToSetting;
        sendAllToStart();
        return;
      }
    }
    if ((leftXVelocity == 128 &&
         rightXVelocity == 128 &&
         rightYVelocity == 128 &&
         isJoystickOwner == true) || !PS3CtrlrHost.IsConnected() == true)
    {
      isJoystickOwner = false;
      joystickSet(false);
    }
  }
  else
  {
    if (prevLeftXVelocity != 128) {
      prevLeftXVelocity = 128;
      setJoystickSpeed(0, 0 );
    }
    if (prevRightXVelocity != 128) {
      prevRightXVelocity = 128;
      setJoystickSpeed(1, 0 );
    }
    
    if(isJoystickOwner == true)
    {
      isJoystickOwner = false;
      joystickSet(false);
    }
  }
}

/** Waiting state

  We wait for motors to finish running in this state

*/
void USBControllerUI::uiStateWait( void )
{
  uint8_t buttonState;
  static uint32_t runningLEDTimer = millis();
  static uint8_t runningLEDState = false, cameraLEDState;

  if (isProgramDone() && (isShotRunning == false || !running) )
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
    uiState = USBCONTROLLERUI_STATE_WaitToStop;;
    StopMove();
    return;
  }

  // Allow user to query the time left using R1/R2
  uint32_t shotTimeLeftMS = (shotTimeMS + leadinTimeMS + leadoutTimeMS) - (millis() - shotStartTime);
  uint8_t timeLeftHours = ((shotTimeLeftMS) / thousand) / (60 * 60);
  uint8_t timeLeftMinutes = ((shotTimeLeftMS) / (60 * thousand)) - (timeLeftHours * 60);

  // Allow settings to be queried while shot is running
  QueryButton( UI_BUTTON_ShotTimeHours, timeLeftMinutes, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_ShotTimeMinutes, timeLeftHours, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_AccelPercentage, uiSettings.accelPercentage, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_DecelPercentage, uiSettings.decelPercentage, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_IntervalTimeS, uiSettings.intervalTimeS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_FocusTimeDS, uiSettings.focusTimeDS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_ExposureTimeS, uiSettings.exposureTimeS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_ExposureTimeDS, uiSettings.exposureTimeDS, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_Leadin, uiSettings.leadinMinutes, FAST_UI_TICK_RATE);
  QueryButton( UI_BUTTON_Leadout, uiSettings.leadoutMinutes, FAST_UI_TICK_RATE);

  // Flash RUNNING LED
  if ((millis() - runningLEDTimer) >= RUNNING_LED_TICK_RATE)
  {
    runningLEDState = !runningLEDState;
    PS3CtrlrHost.SetLED( UI_LED_RUNNING, runningLEDState );
    runningLEDTimer = millis();
  }

  // Flash BUSY LED
  if (Camera.busy())
  {
    if (!cameraLEDState)
    {
      cameraLEDState = true;
      PS3CtrlrHost.SetLED( UI_LED_CAMERA, cameraLEDState);
    }
  }
  else
  {
    if (cameraLEDState)
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
}

/** Start Move

  Set motor and camera parameters and begin the shot/move.

*/
void USBControllerUI::StartMove( void )
{
  uint32_t exposureTime = uiSettings.exposureTimeS * thousand + uiSettings.exposureTimeDS * onehundred;
  uint32_t focusTime = uiSettings.focusTimeS * thousand + uiSettings.focusTimeDS * onehundred;
  uint32_t exposureDelay = uiSettings.exposureWaitS * thousand + uiSettings.exposureWaitDS * onehundred;
  uint32_t intervalTimeMS = uiSettings.intervalTimeS * thousand + uiSettings.intervalTimeDS * onehundred;

  float accelFraction;
  float decelFraction;
  
  if(uiSettings.accelPercentage + uiSettings.decelPercentage > 100)
  {
    // Invalid accel/decel profile
    accelFraction = 0;
    decelFraction = 0;
  }
  else
  {
    accelFraction = uiSettings.accelPercentage / 100.0f;
    decelFraction = uiSettings.accelPercentage / 100.0f;
  }

  shotTimeMS = (60 * (uint32_t) uiSettings.moveTimeMinutes + 60 * 60 * (uint32_t) uiSettings.moveTimeHours);
  shotTimeMS *= thousand;
  leadinTimeMS = 60 * (uint32_t) uiSettings.leadinMinutes;
  leadinTimeMS *= thousand;
  leadoutTimeMS = 60 * (uint32_t) uiSettings.leadoutMinutes;
  leadoutTimeMS *= thousand;

  pingPongMode(false);

  Camera.intervalTime(intervalTimeMS);
  Camera.focusTime( focusTime );
  Camera.triggerTime(exposureTime);
  Camera.delayTime(exposureDelay);

  // Assign motor parameters
  for (uint8_t i = 0; i < USBCONTROLLERUI_NMOTORS; i++) {

    motor[i].clear();
    motor[i].easing(OM_MOT_QUAD);
    motor[i].planType( uiSettings.isContinuous );
    motor[i].planLeadIn( leadinTimeMS / intervalTimeMS );
    motor[i].planLeadOut(leadoutTimeMS / intervalTimeMS);

    if (!uiSettings.isContinuous)
    {
      uint32_t shotCount = shotTimeMS / intervalTimeMS;

      // Number of Shots
      motor[i].planAccelLength(accelFraction * shotCount);
      motor[i].planDecelLength(decelFraction * shotCount);
      motor[i].planTravelLength( shotCount ) ;
      cameraAutoMaxShots();
    }
    else
    {
      // Length of time in MS
      motor[i].planAccelLength( accelFraction * shotTimeMS );
      motor[i].planDecelLength( decelFraction * shotTimeMS );
      motor[i].planTravelLength( shotTimeMS );
    }
    motor[i].planRun();
  }

  shotStartTime = millis();

  isShotRunning = true;
  startProgramCom();
}

/** Waiting to Setting

  Wait for motors to finish moving then resume setting state

*/
void USBControllerUI::uiStateWaitToSetting( void )
{
  uint8_t buttonState = PS3CtrlrHost.GetButtonState(UI_BUTTON_StopShot);

  if ( buttonState == PS3CONTROLLER_STATE_Down)
    buttonTimerStart = millis();

  if ( !areMotorsRunning() )
  {
    uiState = USBCONTROLLERUI_STATE_Setting;
    return;
  }

  if ( (buttonState == PS3CONTROLLER_STATE_On) && ((millis() - buttonTimerStart) > BUTTON_TIMER_TIME) )
  {
    readyToStart = false;
    stopAllMotors();
    uiState = USBCONTROLLERUI_STATE_WaitToStop;
    return;
  }
}

/** Waiting to Stop

  Wait for motors to stop as a result of a stop request

*/
void USBControllerUI::uiStateWaitToStop( void )
{
    for (byte i = 0; i <USBCONTROLLERUI_NMOTORS; i++) {
    if(motor[i].running())
      return;
  }
  uiState = USBCONTROLLERUI_STATE_Setting;
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
  if (buttonState == PS3CONTROLLER_STATE_Down)
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
uint8_t USBControllerUI::MonitorButton( uint8_t modifierButtonState, PS3Controller_ButtonUsages_t button, uint8_t *storeValue, uint16_t queryValue, uint16_t tickRate, uint8_t maxValue )
{
  uint8_t buttonState;
  static uint8_t isQuery = false;
  static uint8_t isMonitoring = false;
  static uint8_t curButtonMonitoring = 0;
  static uint32_t buttonTimer;
  static uint8_t pulseStopped;

  buttonState = PS3CtrlrHost.GetButtonState( button );

  // Only allow one button to be monitored
  if (isMonitoring == false || button == curButtonMonitoring)
  {
    if (buttonState == PS3CONTROLLER_STATE_Down && modifierButtonState == PS3CONTROLLER_STATE_Off)
    {
      curButtonMonitoring = button;
      isMonitoring = true;
      buttonTimer = millis();
      pulseStopped = false;
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
      if(pulseStopped == false)
        ActuatorPulseStop();
      isMonitoring = false;
      *storeValue = actuatorPulseState.nPulses;
      return (true);
    }
    
    if(actuatorPulseState.nPulses >= maxValue && pulseStopped == false)
    {
      ActuatorPulseStop();
      pulseStopped = true;
      actuatorPulseState.nPulses = maxValue;
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
  //PS3CtrlrHost.SetSmallActuator(false, 0x00);
}

/** Iterate Pulses

  Tick pulsing states one frame forward

*/
void USBControllerUI::IteratePulses( void )
{
  uint32_t timeElapsed;
  uint8_t i;
  static uint8_t finishPulse = false;

  if (actuatorPulseState.isOn || finishPulse == true)
  {
    timeElapsed = (millis() - actuatorPulseState.prevUpdateTime);
    if ( actuatorPulseState.offTimeLeft > 0 )
    {
      if (timeElapsed >= actuatorPulseState.offTimeLeft)
      {
        actuatorPulseState.offTimeLeft = 0;
        actuatorPulseState.onTimeLeft = actuatorPulseState.onTime;
        actuatorPulseState.nPulses++;
        finishPulse = true;
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
        finishPulse = false;
      }
      else
        actuatorPulseState.onTimeLeft -= timeElapsed;
    }

    actuatorPulseState.prevUpdateTime = millis();
  }
}

/** Reset UI Defaults

  Reset setting zero/default to the default parameters for a shot.

*/
void USBControllerUI::ResetUIDefaults( void )
{
  uiSettings.moveTimeMinutes = 10;
  uiSettings.moveTimeHours = 0;
  uiSettings.accelPercentage = 0;
  uiSettings.decelPercentage = 0;
  uiSettings.isContinuous = true;
  uiSettings.exposureTimeS = 1;
  uiSettings.exposureTimeDS = 0;
  uiSettings.intervalTimeS = 2;
  uiSettings.focusTimeDS = 0;
  uiSettings.leadinMinutes = 0;
  uiSettings.leadoutMinutes = 0;

  // These are un-modifiable at the moment
  uiSettings.exposureWaitS = 0;
  uiSettings.exposureWaitDS = 2;
  uiSettings.focusTimeS = 0;
  uiSettings.intervalTimeDS = 0;
  
  prevLeftXVelocity = 128;
  prevRightXVelocity = 128;
  prevRightYVelocity = 128;

  PS3CtrlrHost.SetLED( UI_LED_SMS, !uiSettings.isContinuous );
}

/** Save a UI Setting

  Load a setting into the current shots setting.  nSetting must be less than
  USBCONTROLLERUI_NSETTINGS.

*/
void USBControllerUI::SaveUISetting( uint8_t nSetting )
{
  char *tempPtr = (char *) &USBCtrlrUI.uiSettings;

  for (int j = 0; j < sizeof(CtrlrUISettings_t); j++)
  {
    OMEEPROM::write( EE_USBCTRLR_SETTINGS + sizeof(CtrlrUISettings_t)*nSetting + j, tempPtr[j]);
  }
}

/** Load UI Setting

  Load a setting into the current shots setting.  nSetting must be less than
  USBCONTROLLERUI_NSETTINGS.

*/
void USBControllerUI::LoadUISetting( uint8_t nSetting )
{
  char *tempPtr = (char *) &USBCtrlrUI.uiSettings;
  for (int j = 0; j < sizeof(CtrlrUISettings_t); j++)
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
  return (isShotRunning);
}

uint8_t USBControllerUI::isProgramDone( void )
{
  for(uint8_t i=0 ; i<USBCONTROLLERUI_NMOTORS ; i++ )
  {
    if(motor[i].programDone() == false)
      return(false);
  }
  return(true);
}

void USBControllerUI::SetMotorMS( uint8_t motorNum, uint8_t ms )
{
  microstepSettings[motorNum] = ms;
}
