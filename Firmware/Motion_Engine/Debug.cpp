// 
// 
// 

#include "Debug.h"

DebugClass::DebugClass(OMMoCoPrintClass *c_mocoPrint){
	m_mocoPrint = c_mocoPrint;
	init();
}

void DebugClass::init()
{
	/* 
	  Initialize the debug flag to no reporting and 
	  ensure both the debug output lines are off
	*/
	m_debug_flag = B00000000;
	m_usb = true;
    
    // Leaving this here for now, set to false before merging
    m_moco = false;
}

void DebugClass::setState(byte state){
	m_debug_flag = state;
	const char *message = "New debug flag state: ";
	if (m_usb){
		USBSerial.print(message);
		USBSerial.println(state, BIN);
		USBSerial.println("");
	}
	if (m_moco){
		m_mocoPrint->print(message);
		m_mocoPrint->println(state, BIN);
		m_mocoPrint->println("");
	}
}

byte DebugClass::getState(){
	return m_debug_flag;
}

bool DebugClass::setUSB(bool enabled){
	m_usb = enabled;
	return m_usb;
}
bool DebugClass::getUSB(){
	return m_usb;
}

bool DebugClass::setMoco(bool enabled){
	m_moco = enabled;
	return m_moco;
}
bool DebugClass::getMoco(){
	return m_moco;
}
