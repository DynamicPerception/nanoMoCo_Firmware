

/* PS3 Controller Host Arduino LUFA Library

 B. Wiggins / Dynamic Perception LLC

 Based on the excellent work by Dean Camera for his LUFA project.

 His original copyright notice follows.

 New Code License:

 Copyright 2016 B. Wiggins / Dynamic Perception LLC

 Permission to use, copy, modify, distribute, and sell this
 software and its documentation for any purpose is hereby granted
 without fee, provided that the above copyright notice appear in
 all copies and that both that the copyright notice and this
 permission notice and warranty disclaimer appear in supporting
 documentation, and that the name of the author not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.

 The author disclaim all warranties with regard to this
 software, including all implied warranties of merchantability
 and fitness.  In no event shall the author be liable for any
 special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether
 in an action of contract, negligence or other tortious action,
 arising out of or in connection with the use or performance of
 this software.

*/

// Original Copyright Notice from LUFA

/*

 LUFA Library
 Copyright (C) Dean Camera, 2012.

 dean [at] fourwalledcubicle [dot] com
 www.lufa-lib.org
 */

/*
 Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)

 Permission to use, copy, modify, distribute, and sell this
 software and its documentation for any purpose is hereby granted
 without fee, provided that the above copyright notice appear in
 all copies and that both that the copyright notice and this
 permission notice and warranty disclaimer appear in supporting
 documentation, and that the name of the author not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.

 The author disclaim all warranties with regard to this
 software, including all implied warranties of merchantability
 and fitness.  In no event shall the author be liable for any
 special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether
 in an action of contract, negligence or other tortious action,
 arising out of or in connection with the use or performance of
 this software.
 */

/** @file PS3ControllerHost.h

 @brief Core PS3ControllerHost.h Header File
 */

#ifndef _PS3CONTROLLERHOST_H_
#define _PS3CONTROLLERHOST_H_

/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/interrupt.h>


#if ! defined(USB_CAN_BE_BOTH) || ! defined(USB_CAN_BE_HOST)
#error The current board does not support host mode
#endif

//extern "C" {
//    extern USB_ClassInfo_HID_Host_t PS3Controller_HID_Interface;
//}


/** PS3ControllerHost Class

 This class provides high-level access to an attached PS3 Controller device when used with a
 board that support OTG or USB Host mode.  Based on the excellent keyboard host and LUFA
 code provided by Dean Camera.

 This class is designed to be used within the Arduino IDE, using the AT90USB Arduino hardware
 extension, and can only be used with supported board types, including, but not limited to:

 <ul>
    <li>AT90USB647</li>
    <li>AT90USB1287</li>

 @author
 B. Wiggins

 @copyright
 Copyright &copy; 2016 Dynamic Perception LLC, with large contributions from work Copyright &copy; 2012 Dean Camera
 */


#define NUMBER_OF_BUTTONS (17)
#define REPORT_BUF_SIZE (64)

enum PS3Controller_ButtonStates_t
{
  PS3CONTROLLER_STATE_Off,  // button is not pressed
  PS3CONTROLLER_STATE_On,   // button is pressed
  PS3CONTROLLER_STATE_Up,   // button just transitioned from pressed to not pressed
  PS3CONTROLLER_STATE_Down, // button just transitioned from not pressed to pressed
};

enum PS3Controller_ButtonUsages_t
{
  PS3CONTROLLER_BUTTON_Select   = 1,
  PS3CONTROLLER_BUTTON_L3       = 2,
  PS3CONTROLLER_BUTTON_R3       = 3,
  PS3CONTROLLER_BUTTON_Start    = 4,
  PS3CONTROLLER_BUTTON_Up       = 5,
  PS3CONTROLLER_BUTTON_Right    = 6,
  PS3CONTROLLER_BUTTON_Down     = 7,
  PS3CONTROLLER_BUTTON_Left     = 8,
  PS3CONTROLLER_BUTTON_L2       = 9,
  PS3CONTROLLER_BUTTON_R2       = 10,
  PS3CONTROLLER_BUTTON_L1       = 11,
  PS3CONTROLLER_BUTTON_R1       = 12,
  PS3CONTROLLER_BUTTON_Triangle = 13,
  PS3CONTROLLER_BUTTON_Circle   = 14,
  PS3CONTROLLER_BUTTON_Cross    = 15,
  PS3CONTROLLER_BUTTON_Square   = 16,
  PS3CONTROLLER_BUTTON_PS       = 17,
};

class PS3ControllerHost {

  public:
    bool isConnected;
    // ctors
    PS3ControllerHost();
    void USBTask(void);
    PS3Controller_ButtonStates_t GetButtonState( PS3Controller_ButtonUsages_t button );  // Get button state and set latching info
    uint8_t GetLeftStickX();
    uint8_t GetLeftStickY();
    uint8_t GetRightStickX();
    uint8_t GetRightStickY();
    void SetLED( uint8_t LEDNum, bool state ); // Set LED: 1-4 true/false
    void SetBigActuator( uint8_t rumbleValue,  uint8_t rumbleDuration ); // 0-255
    void SetSmallActuator( bool rumbleState, uint8_t rumbleDuration ); // 0-1
    void SetLEDPulse( uint8_t LEDNum, uint16_t onMS, uint16_t offMS, uint8_t nTimes);
    void init( void );
    void ResetControllerState( void );
    
    // Remove features we aren't using to save memory
    PS3Controller_ButtonStates_t PeekButtonState( PS3Controller_ButtonUsages_t button ); // Get button state without setting latching info
    uint8_t GetButtonPressure( PS3Controller_ButtonUsages_t button );

  private:
    uint32_t curDigitalButtons;
    uint32_t prevDigitalButtons;
    uint8_t LEDStates;
    uint8_t BigActuator;
    uint8_t BigActuatorDuration;
    bool SmallActuator;
    uint8_t SmallActuatorDuration;
    uint8_t inputReportBuf[REPORT_BUF_SIZE];
    
    // Remove features we aren't using to save memory
    //uint16_t accelX, accelY, accelZ, accelGyro;
};

extern PS3ControllerHost PS3CtrlrHost;


/* C Function Prototypes From LUFA */

void EVENT_USB_Host_HostError(const uint8_t p_err);
void EVENT_USB_Host_DeviceAttached(void);
void EVENT_USB_Host_DeviceUnattached(void);
void EVENT_USB_Host_DeviceEnumerationFailed(const uint8_t p_err, const uint8_t p_suberr);
void EVENT_USB_Host_DeviceEnumerationComplete(void);
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const p_item);

#endif // _PS3CONTROLLERHOST_H_


