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

#define USBCONTROLLERUI_NLEDS (4)
#define USBCONTROLLERUI_NACTUATORS (2)

typedef struct {
  bool isOn;
  uint8_t LEDNum;
  uint8_t pulsesLeft;
  uint16_t onTime;
  uint16_t offTime;
  uint16_t onTimeLeft;
  uint16_t offTimeLeft;
  uint32_t prevUpdateTime;
} LEDPulseState_t;

typedef struct {
  bool isOn;
  uint8_t ActNum;
  uint8_t intensity;
  uint8_t pulsesLeft;
  uint16_t onTime;
  uint16_t offTime;
  uint16_t onTimeLeft;
  uint16_t offTimeLeft;
  uint32_t prevUpdateTime;
} ActuatorPulseState_t;

enum USBControllerUI_State_t
{
  USBCONTROLLERUI_STATE_Setting,
  USBCONTROLLERUI_STATE_Wait,
  USBCONTROLLERUI_STATE_WaitToStart,
};

class USBControllerUI {
  private:

    USBControllerUI_State_t state;
    bool prevStatus;
    bool isShotRunning;
    float prevLeftXVelocity;
    float prevLeftYVelocity;
    float prevRightXVelocity;
    float prevRightYVelocity;

    uint8_t moveTimeMinutes;
    uint8_t moveTimeHours;
    uint8_t accelTime;  // Percent of the time of the move that the dolly is accelerating (0-100)
    uint8_t decelTime;  // Percent of the time of the move that the dolly is decelerating (0-100)
    uint32_t shotStartTime;
    uint32_t shotTimeMS;
    uint8_t isContinuous;
    uint16_t exposureTimeS;
    uint16_t exposureWaitS;
    uint16_t exposureTimeDS;
    uint16_t exposureWaitDS;
    uint32_t killTimerStart;
    
    float topSpeed;
    
     
    // LED state information for UI Feedback
    LEDPulseState_t LEDPulseState;

    // Actuator state information for UI Feedback
    ActuatorPulseState_t actuatorPulseState;

    //uint32_t buttonTimers[17];
    void StartMove( void );

    void StateSetting( void );
    void StateWait( void );
    void StateWaitToStart( void );

    float CreateDeadzone( float value );
    void IteratePulses( void );

    // UI Indicator functions
    void LEDPulse( uint8_t LEDNum, uint16_t onMS, uint16_t offMS, uint8_t nTimes);
    void ActuatorPulse( uint8_t ActNum, uint8_t intensity, uint16_t onMS, uint16_t offMS, uint8_t nTimes);
    void LEDPulseStop( uint8_t LEDNum );
    void ActuatorPulseStop( void );

    // User Input functions
    bool MonitorButton( PS3Controller_ButtonUsages_t modifierButton, PS3Controller_ButtonUsages_t button, uint32_t *storeMs, uint16_t queryValue );

  public:
    USBControllerUI( void );
    void init( void );
    void UITask( void );
};

extern USBControllerUI USBCtrlrUI;

#ifndef _USBCONTROLLERUI_H_
#define _USBCONTROLLERUI_H_


#endif // _USBCONTROLLERUI_H_


