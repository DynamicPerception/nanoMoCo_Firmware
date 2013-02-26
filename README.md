Motion Engine Firmware for nanoMoCo / Arduino
=============================================

Version: 1.0.7
---------------

This is the Motion Engine for the Arduino/nanoMoCo Platform.

Version: 1.0.7

### Required Libraries

You must install the following libraries to compile the MotionEngine:

 * [OMLibraries v1.13](https://github.com/DynamicPerception/OMLibraries/tree/v1.13) 
 * [MsTimer2](http://www.pjrc.com/teensy/td_libs_MsTimer2.html)
 * [TimerOne](http://code.google.com/p/arduino-timerone/downloads/list)


For more information on installing libraries in Arduino, see [Arduino Libraries Guide](http://arduino.cc/en/Guide/Libraries).

### Uploading to nanoMoCo

Key tips for uploading to the nanoMoCo:

 * Ensure you select 'Arduino Uno' as the Board Type
 * You have 8 seconds after powering on the nanoMoCo to begin uploading firmware
 * You may only upload firmware over the RS485 interface

### Bootloader

This bundle includes a bootloader for the nanoMoCo, or any ATMega328p device, to upload firmware over RS485.  Play with at will.

### More Information
 
More information can be found at http://www.dynamicperception.com/

### Copyright and License

(c) 2008-2013 C.A. Church / Dynamic Perception LLC

The Motion Engine firmware is distributed under a GPLv3 License. See LICENSE.txt for licensing information.

