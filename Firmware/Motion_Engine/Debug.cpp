// 
// 
// 

#include "Debug.h"

byte DebugClass::usb_debug = B00000000;

DebugClass::DebugClass(){
	init();
}

void DebugClass::init()
{
	
}

void DebugClass::setState(byte state){
	usb_debug = state;
}

byte DebugClass::getState(){
	return usb_debug;
}

DebugClass Debug;

