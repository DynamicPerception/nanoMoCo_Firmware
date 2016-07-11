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

static bool isPS3Controller;
static uint32_t digitalButtonStates;
static HID_ReportInfo_t HIDReportInfo;

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */

USB_ClassInfo_HID_Host_t PS3Controller_HID_Interface

= {
  /* .Config = */ {
    /* .DataINPipe             = */ {
      /* .Address        = */ (PIPE_DIR_IN  | 1),
      /* .Banks          = */ false,
    },
    /* .DataOUTPipe             = */ {
      /* .Address        = */ (PIPE_DIR_OUT | 2),
      /* .Banks          = */ false,
    },
    /* .HIDInterfaceProtocol   = */ HID_CSCP_NonBootProtocol,

    &HIDReportInfo,
  },
};

void PS3ControllerHost::init( void )
{
  isConnected=false;
}

void PS3ControllerHost::USBTask(void)
{
  if ((USB_HostState != HOST_STATE_Configured) || !(PS3Controller_HID_Interface.State.IsActive))
    return;

  if (HID_Host_IsReportReceived(&PS3Controller_HID_Interface))
  {
    HID_Host_ReceiveReport(&PS3Controller_HID_Interface, inputReportBuf);

    // Copy digital button data
    uint32_t *buttonPtr = (uint32_t *) &inputReportBuf[2];
    curDigitalButtons = *buttonPtr;
/*
    // Copy analog stick data
    leftStick[0] = inputReportBuf[6];
    leftStick[1] = inputReportBuf[7];
    rightStick[0] = inputReportBuf[8];
    rightStick[1] = inputReportBuf[9];

    // Copy pressure sensitive data
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
      analogButtons[i] = PS3ControllerReport[13 + i];
    }

       // Copy accelerometer data, these are 10-bit
       uint16_t *accelPtr;
       accelPtr = (uint16_t *) &PS3ControllerReport[40];
       accelX = *accelPtr;
       accelPtr = (uint16_t *) &PS3ControllerReport[42];
       accelY = *accelPtr;
       accelPtr = (uint16_t *) &PS3ControllerReport[44];
       accelZ = *accelPtr;
       accelPtr = (uint16_t *) &PS3ControllerReport[46];
       accelGyro = *accelPtr;*/
  }

  HID_Host_USBTask(&PS3Controller_HID_Interface);
}

PS3ControllerHost::PS3ControllerHost() {

}

void PS3ControllerHost::ResetControllerState( void )
{
  BigActuator = 0;
  BigActuatorDuration = 0;
  SmallActuator = false;
  SmallActuatorDuration = 0;
}

PS3Controller_ButtonStates_t PS3ControllerHost::GetButtonState( PS3Controller_ButtonUsages_t button )
{
  bool curButtonOn = (curDigitalButtons & (1 << button - 1)) > 0;
  bool prevButtonOn = (prevDigitalButtons & (1 << button - 1)) > 0;

  // set previous state for latching
  if (curButtonOn == true )
    prevDigitalButtons |= 1 << (button - 1);
  else
    prevDigitalButtons &= ~(1 << (button - 1));

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
  bool curButtonOn = (curDigitalButtons & (1 << button - 1)) > 0;
  bool prevButtonOn = (prevDigitalButtons & (1 << button - 1)) > 0;

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

uint8_t PS3ControllerHost::GetButtonPressure( PS3Controller_ButtonUsages_t button )
{
  return(inputReportBuf[button + 12]);
}

uint8_t PS3ControllerHost::GetLeftStickX() {
  return (inputReportBuf[6]);
}

uint8_t PS3ControllerHost::GetLeftStickY() {
  return (inputReportBuf[7]);
}

uint8_t PS3ControllerHost::GetRightStickX() {
  return (inputReportBuf[8]);
}

uint8_t PS3ControllerHost::GetRightStickY() {
  return (inputReportBuf[9]);
}

static uint8_t LEDRUMReport [] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x02, 0xff, 0x27, 0x10, 0x00, 0x32, 0xff,
                                  0x27, 0x10, 0x00, 0x32, 0xff, 0x27, 0x10, 0x00,
                                  0x32, 0xff, 0x27, 0x10, 0x00, 0x32, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                                 };
                             
void PS3ControllerHost::SetLEDPulse( uint8_t LEDNum, uint16_t onMS, uint16_t offMS, uint8_t nTimes)
{
  LEDStates |= 1 << (LEDNum);

  LEDRUMReport[9] = LEDStates;
  
  LEDRUMReport[10+(LEDNum-1)*5] = 0xFE;                 // Total time enabled 
  LEDRUMReport[10+((LEDNum-1)*5)+1] = (onMS+offMS)/100;  // Duty cycle in deciseconds
  LEDRUMReport[10+((LEDNum-1)*5)+2] = 0xFF;             // Enabled
  LEDRUMReport[10+((LEDNum-1)*5)+3] = 255.0f*((float) offMS/((float) (onMS+offMS)));  // Duty off (percentage of duty cycle normalized to 0-255)
  LEDRUMReport[10+((LEDNum-1)*5)+4] = 255.0f*((float) onMS/((float) (onMS+offMS)));   // Duty on (percentage of duty cycle normalized to 0-255)
  
  //char szTemp[32];
  //sprintf(szTemp, "%d %d %d %d %d", LEDRUMReport[10+(LEDNum-1)*5], LEDRUMReport[10+((LEDNum-1)*5)+1],LEDRUMReport[10+((LEDNum-1)*5)+2],LEDRUMReport[10+((LEDNum-1)*5)+3],LEDRUMReport[10+((LEDNum-1)*5)+4]);
  //debug.functln(szTemp);
  HID_Host_SendReportByID(&PS3Controller_HID_Interface, 0x01, HID_REPORT_ITEM_Out, LEDRUMReport, sizeof(LEDRUMReport));
}

void PS3ControllerHost::SetLED( uint8_t LEDNum, bool state )
{
  if (state == true)
    LEDStates |= 1 << (LEDNum);
  else
    LEDStates &= ~(1 << (LEDNum));
    
  LEDRUMReport[9] = LEDStates;
  //LEDRUMReport[10+((LEDNum-1)*5)+2]=0;
  
  HID_Host_SendReportByID(&PS3Controller_HID_Interface, 0x01, HID_REPORT_ITEM_Out, LEDRUMReport, sizeof(LEDRUMReport));
}

void PS3ControllerHost::SetBigActuator( uint8_t rumbleValue, uint8_t rumbleDuration )
{
  BigActuator =  rumbleValue;
  BigActuatorDuration = rumbleDuration;
      
  LEDRUMReport[1] = SmallActuatorDuration;
  
  if(SmallActuator == true)
    LEDRUMReport[2] = 0xff;
  else
    LEDRUMReport[2] = 0x00;
  
  LEDRUMReport[3] = BigActuatorDuration;
  LEDRUMReport[4] = BigActuator;
  
  // The same packet controls LED settings so make sure they stay the same
  LEDRUMReport[9] = LEDStates;

  HID_Host_SendReportByID(&PS3Controller_HID_Interface, 0x01, HID_REPORT_ITEM_Out, LEDRUMReport, sizeof(LEDRUMReport));
}

void PS3ControllerHost::SetSmallActuator( bool rumbleState, uint8_t rumbleDuration )
{
  SmallActuator = rumbleState;
  SmallActuatorDuration = rumbleDuration;

  LEDRUMReport[1] = SmallActuatorDuration;
 
  if(SmallActuator == true)
    LEDRUMReport[2] = 0xff;
  else
    LEDRUMReport[2] = 0x00;
    
  LEDRUMReport[3] = BigActuatorDuration;
  LEDRUMReport[4] = BigActuator;

  // The same packet controls LED settings so make sure they stay the same
  LEDRUMReport[9] = LEDStates;
  
  HID_Host_SendReportByID(&PS3Controller_HID_Interface, 0x01, HID_REPORT_ITEM_Out, LEDRUMReport, sizeof(LEDRUMReport));
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
  PS3CtrlrHost.isConnected=false;
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
  if (DeviceDescriptor.VendorID == PS3CONTROLLER_VID && DeviceDescriptor.ProductID == PS3CONTROLLER_PID)
    isPS3Controller = true;
  else
    isPS3Controller = false;

  if (HID_Host_ConfigurePipes(&PS3Controller_HID_Interface, ConfigDescriptorSize, ConfigDescriptorData) != HID_ENUMERROR_NoError) {
    // Hard fail
    return;
  }

  if (USB_Host_SetDeviceConfiguration(1) != HOST_SENDCONTROL_Successful) {
    // Hard fail
    return;
  }

  if (!PS3Controller_HID_Interface.State.IsActive)
    return;


  uint8_t ret;
  ret = HID_Host_SetReportProtocol(&PS3Controller_HID_Interface);
  if (ret != HOST_SENDCONTROL_Successful )
  {
    // Hard fail
    return;
  }

  if (!(HIDReportInfo.TotalReportItems))
  {
    // Hard fail
    return;
  }

  if (isPS3Controller)
  {
    // Code from Dean's PS3 Controller driver to send the bluetooth address to the PS3 controller for pairing (we should implement something similar)
#ifdef PS3BLUETOOTH
    /* Read out the latest inserted bluetooth adapter address stored in EEPROM */
    BDADDR_t TempAddress;
    //eeprom_read_block(TempAddress, BluetoothAdapter_LastLocalBDADDR, sizeof(BDADDR_t));
    TempAddress = (BDADDR_t) device_address;
    /* Send PS3 bluetooth host pair request report to the adapter */
    uint8_t PS3AdapterPairRequest[]    = {0x01, 0x00, TempAddress[5], TempAddress[4], TempAddress[3], TempAddress[2], TempAddress[1], TempAddress[0]};
    HID_Host_SendReportByID(&PS3Controller_HID_Interface, 0xF5, HID_REPORT_ITEM_Feature, PS3AdapterPairRequest, sizeof(PS3AdapterPairRequest));
#endif
    //debug.functln("Setting Report Request");
    /* Instruct the PS3 controller to send reports via the HID data IN endpoint */
    uint8_t PS3StartReportingRequest[] = {0x42, 0x0C, 0x00, 0x00};
    HID_Host_SendReportByID(&PS3Controller_HID_Interface, 0xF4, HID_REPORT_ITEM_Feature, PS3StartReportingRequest, sizeof(PS3StartReportingRequest));
  }

  if (isPS3Controller)
  {
    PS3CtrlrHost.ResetControllerState();
    PS3CtrlrHost.isConnected=true;
  }
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


/** Callback for the HID Report Parser. This function is called each time the HID report parser is about to store
 *  an IN, OUT or FEATURE item into the HIDReportInfo structure. To save on RAM, we are able to filter out items
 *  we aren't interested in (preventing us from being able to extract them later on, but saving on the RAM they would
 *  have occupied).
 *
 *  \param[in] CurrentItem  Pointer to the item the HID report parser is currently working with
 *
 *  \return Boolean true if the item should be stored into the HID report structure, false if it should be discarded
 */
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem)
{
  bool IsJoystick = false;

  /* Iterate through the item's collection path, until either the root collection node or a collection with the
   * Joystick Usage is found - this distinguishes joystick HID devices from mouse devices
   */
  for (HID_CollectionPath_t* CurrPath = CurrentItem->CollectionPath; CurrPath != NULL; CurrPath = CurrPath->Parent)
  {
    if (CurrPath->Usage.Usage == USAGE_JOYSTICK)
    {
      IsJoystick = true;
      break;
    }
  }

  if (isPS3Controller)
  {
    if (CurrentItem->Attributes.Usage.Page == USAGE_PAGE_BUTTON)
    {
      /* Map button usages to functions suitable for a PS3 Controller */
      switch (CurrentItem->Attributes.Usage.Usage)
      {
          // Just to keep the HID driver happy
        case PS3CONTROLLER_BUTTON_PS:
          return true;
      }
    }
  }
  return false;
}


