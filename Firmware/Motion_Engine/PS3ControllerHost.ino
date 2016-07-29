/* PS3 Controller Host Arduino LUFA Library

B. Wiggins / Dynamic Perception LLC

Based on the excellent work by Dean Camera for his LUFA project
and bluetooth robot project.

Shared under the MIT License

*/

#include "PS3ControllerHost.h"
#include "Debug.h"

/** HID Report Descriptor Usage Page value for a toggle button. */
#define USAGE_PAGE_BUTTON           0x09

/** HID Report Descriptor Usage Page value for a Generic Desktop Control. */
#define USAGE_PAGE_GENERIC_DCTRL    0x01

/** HID Report Descriptor Usage for a Joystick. */
#define USAGE_JOYSTICK              0x04

/** Device Vendor ID value for the PS3 Controller. */
#define PS3CONTROLLER_VID           0x054C

/** Device Product ID value for the PS3 Controller. */
#define PS3CONTROLLER_PID           0x0268

#define PS3_CONTROL_PIPE        0
#define PS3_OUTPUT_PIPE         2
#define PS3_INPUT_PIPE          1

static uint32_t digitalButtonStates;

void PS3Ctrlr_Host_ConfigurePipes( void )
{
  uint8_t ret;

  // Save memory by hard coding the configuration
  Pipe_ConfigurePipe( PS3_INPUT_PIPE | PIPE_DIR_IN , EP_TYPE_INTERRUPT, 0x01, 64, 0 );
  Pipe_ConfigurePipe( PS3_OUTPUT_PIPE, EP_TYPE_INTERRUPT, 0x02, 64, 0 );
}

void PS3ControllerHost::init( void )
{
  isConnected = false;
}
#define DEADZONE 10
void PS3ControllerHost::USBTask(void)
{
  if  (USB_HostState != HOST_STATE_Configured || !isConnected )
    return;

  if (GetReportData((char *) inputReportBuf, 48))
  {
    // Copy digital button data
    memcpy(&curDigitalButtons, &inputReportBuf[2], sizeof(uint32_t));
    LeftStickX = inputReportBuf[6];
    LeftStickY = inputReportBuf[7];
    RightStickX = inputReportBuf[8];
    RightStickY = inputReportBuf[9];
  }
}

PS3ControllerHost::PS3ControllerHost() {

}

uint8_t PS3ControllerHost::IsConnected( void )
{
  return (isConnected);
}

void PS3ControllerHost::ResetControllerState( void )
{
  SmallActuator = false;
  SmallActuatorDuration = 0;
}

const uint32_t bitConst = 0x0001;

PS3Controller_ButtonStates_t PS3ControllerHost::GetButtonState( PS3Controller_ButtonUsages_t button )
{
  if (!isConnected)
    return (PS3CONTROLLER_STATE_Off);

  uint8_t curButtonOn;
  uint8_t prevButtonOn;

  curButtonOn = (curDigitalButtons & (bitConst << (button - 1))) > 0;
  prevButtonOn = (prevDigitalButtons & (bitConst << (button - 1))) > 0;

  // set previous state for latching
  if (curButtonOn == true )
    prevDigitalButtons |= bitConst << (button - 1);
  else
    prevDigitalButtons &= ~(bitConst << (button - 1));

  if (curButtonOn == true && prevButtonOn == true)
    return (PS3CONTROLLER_STATE_On);
  else if (curButtonOn == true && prevButtonOn == false)
    return (PS3CONTROLLER_STATE_Down);
  else if (curButtonOn == false && prevButtonOn == true)
    return (PS3CONTROLLER_STATE_Up);
  else // curButtonOff && prevButtoOff
    return (PS3CONTROLLER_STATE_Off);
}

PS3Controller_ButtonStates_t PS3ControllerHost::PeekButtonState( PS3Controller_ButtonUsages_t button )
{
  if (!isConnected)
    return (PS3CONTROLLER_STATE_Off);

  uint8_t curButtonOn;
  uint8_t prevButtonOn;

  curButtonOn = (curDigitalButtons & (bitConst << button - 1)) > 0;
  prevButtonOn = (prevDigitalButtons & (bitConst << button - 1)) > 0;

  if (curButtonOn == true && prevButtonOn == true)
  {
    return (PS3CONTROLLER_STATE_On);
  }
  else if (curButtonOn == true && prevButtonOn == false)
  {
    return (PS3CONTROLLER_STATE_Down);
  }
  else if (curButtonOn == false && prevButtonOn == true)
  {
    return (PS3CONTROLLER_STATE_Up);
  }
  else // curButtonOff && prevButtoOff
  {
    return (PS3CONTROLLER_STATE_Off);
  }
}

/*
uint8_t PS3ControllerHost::GetButtonPressure( PS3Controller_ButtonUsages_t button )
{
  return (inputReportBuf[button + 12]);
}
*/

static uint8_t LEDRUMReport [] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x02, 0xff, 0x27, 0x10, 0x00, 0x32, 0xff,
                                  0x27, 0x10, 0x00, 0x32, 0xff, 0x27, 0x10, 0x00,
                                  0x32, 0xff, 0x27, 0x10, 0x00, 0x32, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                                 };

void PS3ControllerHost::SetLEDPulse( uint8_t LEDNum, uint16_t onMS, uint16_t offMS, uint8_t nTimes)
{
  if (!isConnected)
    return;

  LEDStates |= 1 << (LEDNum);

  LEDRUMReport[9] = LEDStates;

  LEDRUMReport[10 + (LEDNum - 1) * 5] = 0xFE;           // Total time enabled
  LEDRUMReport[10 + ((LEDNum - 1) * 5) + 1] = (onMS + offMS) / 100; // Duty cycle in deciseconds
  LEDRUMReport[10 + ((LEDNum - 1) * 5) + 2] = 0xFF;     // Enabled
  LEDRUMReport[10 + ((LEDNum - 1) * 5) + 3] = 255.0f * ((float) offMS / ((float) (onMS + offMS))); // Duty off (percentage of duty cycle normalized to 0-255)
  LEDRUMReport[10 + ((LEDNum - 1) * 5) + 4] = 255.0f * ((float) onMS / ((float) (onMS + offMS))); // Duty on (percentage of duty cycle normalized to 0-255)

  SendReportData((char *) LEDRUMReport, sizeof(LEDRUMReport));
}

uint8_t SendReportData( char *data, uint8_t datasize )
{
  USB_ControlRequest.bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS  | REQREC_INTERFACE);
  USB_ControlRequest.bRequest =  HID_REQ_SetReport;

  USB_ControlRequest.wValue =  ((HID_REPORT_ITEM_Out + 1) << 8) | 0x01;
  USB_ControlRequest.wIndex = 0;
  USB_ControlRequest.wLength = 48;

  Pipe_SelectPipe(PIPE_CONTROLPIPE);

  uint8_t ret;
  if ((ret = USB_Host_SendControlRequest(data)) != HOST_SENDCONTROL_Successful)
    return false;

  return (true);
}

uint8_t GetReportData( char *dataBuf, uint8_t datasize )
{

  uint8_t ErrorCode;
  Pipe_SelectPipe(PS3_INPUT_PIPE);
  Pipe_Unfreeze();
  if (!Pipe_IsINReceived() )
  {
    Pipe_Freeze();
    return (false);
  }
  uint16_t ReportSize;

  uint8_t* BufferPos = (uint8_t *) dataBuf;

  ReportSize = Pipe_BytesInPipe();

  if (Pipe_Read_Stream_LE(BufferPos, ReportSize, NULL))
    return false;

  Pipe_ClearIN();
  Pipe_Freeze();
  return true;
}

void PS3ControllerHost::SetLED( uint8_t LEDNum, uint8_t state )
{
  if (!isConnected)
    return;

  if (state == true)
    LEDStates |= 1 << (LEDNum);
  else
    LEDStates &= ~(1 << (LEDNum));

  LEDRUMReport[9] = LEDStates;
  //LEDRUMReport[10+((LEDNum-1)*5)+2]=0;

  SendReportData((char *) LEDRUMReport, sizeof(LEDRUMReport));
}

void PS3ControllerHost::SetSmallActuator( uint8_t rumbleState, uint8_t rumbleDuration )
{
  if (!isConnected)
    return;

  SmallActuator = rumbleState;
  SmallActuatorDuration = rumbleDuration;

  LEDRUMReport[1] = SmallActuatorDuration;

  if (SmallActuator == true)
    LEDRUMReport[2] = 0xff;
  else
    LEDRUMReport[2] = 0x00;

  LEDRUMReport[3] = 0;
  LEDRUMReport[4] = 0;

  // The same packet controls LED settings so make sure they stay the same
  LEDRUMReport[9] = LEDStates;

  SendReportData((char *) LEDRUMReport, sizeof(LEDRUMReport));
}

// create an object, we'll need it below
PS3ControllerHost PS3CtrlrHost = PS3ControllerHost();

/** Event handler for the USB_DeviceAttached event. This indicates that a device has been attached to the host, and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Host_DeviceAttached(void) {
}

/** Event handler for the USB_DeviceUnattached event. This indicates that a device has been removed from the host, and
 *  stops the library USB task management process.
 */
void EVENT_USB_Host_DeviceUnattached(void) {
  PS3CtrlrHost.isConnected = false;
}

/** Event handler for the USB_DeviceEnumerationComplete event. This indicates that a device has been successfully
 *  enumerated by the host and is now ready to be used by the application.
 */

void EVENT_USB_Host_DeviceEnumerationComplete(void) {

  USB_Descriptor_Device_t DeviceDescriptor;
  uint16_t ConfigDescriptorSize;
  uint8_t  ConfigDescriptorData[512];

  if (USB_Host_GetDeviceDescriptor(&DeviceDescriptor) != HOST_SENDCONTROL_Successful)
  {
    // Hard fail
    return;
  }

  if (USB_Host_GetDeviceConfigDescriptor(1, &ConfigDescriptorSize, ConfigDescriptorData, sizeof(ConfigDescriptorData)) != HOST_GETCONFIG_Successful) {
    // Hard fail
    return;
  }

  // Check if this is a PS3 Controller
  if (!(DeviceDescriptor.VendorID == PS3CONTROLLER_VID && DeviceDescriptor.ProductID == PS3CONTROLLER_PID))
    return;

  if (USB_Host_SetDeviceConfiguration(1) != HOST_SENDCONTROL_Successful)
  {
    // Hard fail
    return;
  }

  PS3Ctrlr_Host_ConfigurePipes();

  uint8_t PS3StartReportingRequest[] = {0x42, 0x0C, 0x00, 0x00};

  USB_ControlRequest.bmRequestType = (REQDIR_HOSTTODEVICE | REQTYPE_CLASS  | REQREC_INTERFACE);
  USB_ControlRequest.bRequest =  HID_REQ_SetReport;
  USB_ControlRequest.wValue =  ((HID_REPORT_ITEM_Feature + 1) << 8) | 0xF4;

  USB_ControlRequest.wIndex = 0;
  USB_ControlRequest.wLength = 4;
  Pipe_SelectPipe(PIPE_CONTROLPIPE);
  
  if (USB_Host_SendControlRequest(PS3StartReportingRequest)!=0)
  {
    // Hard fail
    return;
  }
  PS3CtrlrHost.ResetControllerState();
  PS3CtrlrHost.isConnected = true;

  debug.functln("CTRLR CONNECT");
}


/** Event handler for the USB_HostError event. This indicates that a hardware error occurred while in host mode. */
void EVENT_USB_Host_HostError(const uint8_t p_err) {
  // TODO: Figure out if there is a way to fix this or if the PS3 controller just draws too much power for OTG
  // HOST_ERROR_VBusVoltageDip is the only possible error
  USB_Disable();

  // Hard fail
}

/** Event handler for the USB_DeviceEnumerationFailed event. This indicates that a problem occurred while
 *  enumerating an attached USB device.
 */
void EVENT_USB_Host_DeviceEnumerationFailed(const uint8_t p_err, const uint8_t p_suberr) {
  // Hard fail
}

