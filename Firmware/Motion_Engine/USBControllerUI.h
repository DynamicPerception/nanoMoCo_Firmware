/*

USBControllerUI

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

/** @file USBControllerUI.h

 @brief Core USBControllerUI.h Header File
 */
 
#define USBCONTROLLERUI_NSETTINGS (3)
#define USBCONTROLLERUI_NLEDS (4)
#define USBCONTROLLERUI_NACTUATORS (2)

typedef struct {
  uint8_t isOn;
  uint8_t LEDNum;
  uint8_t pulsesLeft;
  uint16_t onTime;
  uint16_t offTime;
  uint16_t onTimeLeft;
  uint16_t offTimeLeft;
  uint32_t prevUpdateTime;
} LEDPulseState_t;

typedef struct {
  uint8_t isOn;
  uint8_t ActNum;
  uint8_t intensity;
  uint8_t pulsesLeft;
  uint8_t nPulses;
  uint16_t onTime;
  uint16_t offTime;
  uint16_t onTimeLeft;
  uint16_t offTimeLeft;
  uint32_t prevUpdateTime;
} ActuatorPulseState_t;

typedef struct {
    uint8_t isContinuous;
    uint8_t moveTimeMinutes;
    uint8_t moveTimeHours;
    uint8_t accelPercentage;  // Percent of the time of the move that the dolly is accelerating (0-100)
    uint8_t decelPercentage;  // Percent of the time of the move that the dolly is decelerating (0-100)
    uint8_t intervalTimeS;
    uint8_t focusTimeDS;
    uint8_t exposureTimeS;
    uint8_t exposureWaitS;
    uint8_t exposureTimeDS;
    uint8_t leadinMinutes;
    uint8_t leadinHours;
    uint8_t intervalTimeDS;
   
    // Un-modifiable at this time
    uint8_t focusTimeS;
    uint8_t exposureWaitDS;
} CtrlrUISettings_t; 

enum USBControllerUI_State_t
{
  USBCONTROLLERUI_STATE_Setting,
  USBCONTROLLERUI_STATE_Wait,
  USBCONTROLLERUI_STATE_WaitToStart,
};

class USBControllerUI {
  private:

    USBControllerUI_State_t uiState;
    uint8_t prevStatus;
    uint8_t isShotRunning;
    uint8_t  prevLeftXVelocity;
    uint8_t  prevLeftYVelocity;
    uint8_t prevRightXVelocity;
    uint8_t  prevRightYVelocity;

    uint32_t shotStartTime;
    uint32_t shotTimeMS;
    uint32_t leadinTimeMS;
   
    uint32_t buttonTimerStart;

    // Actuator state information for UI Feedback
    ActuatorPulseState_t actuatorPulseState;
    
    void StartMove( void );
    void StopMove( void );
    void TriggerStartMove( void );

    void uiStateSetting( void );
    void uiStateWait( void );
    void uiStateWaitToStart( void );

    void IteratePulses( void );

    // UI Indicator functions
    void ActuatorPulse( uint16_t onMS, uint16_t offMS, uint8_t nTimes);
    void ActuatorPulseStop( void );
    void ConfirmPulse( void );

    // User Input functions
    uint8_t MonitorButton( uint8_t modifierButtonState, PS3Controller_ButtonUsages_t button, uint8_t *storeValue, uint16_t queryValue, uint16_t tickRate );
    void QueryButton( PS3Controller_ButtonUsages_t button, uint16_t queryValue, uint16_t tickRate);
    void ResetUIDefaults( void );
    void SaveUISetting( uint8_t nSetting );
    void LoadUISetting( uint8_t nSetting );
    float CreateDeadzone( float value );

  public:
    USBControllerUI( void );
    void init( void );
    void UITask( void );
    uint8_t IsShotRunning( void );
    CtrlrUISettings_t uiSettings;
};

extern USBControllerUI USBCtrlrUI;

#ifndef _USBCONTROLLERUI_H_
#define _USBCONTROLLERUI_H_


#endif // _USBCONTROLLERUI_H_



