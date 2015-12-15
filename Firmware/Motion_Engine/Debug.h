// Debug.h

#ifndef _DEBUG_h
#define _DEBUG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class DebugClass
{
 private:
	 void init();
	 static byte  usb_debug;	// Byte holding debug output flags

 public:
	DebugClass();		
	const static byte DB_COM_OUT	= B00000001;	// Debug flag -- toggles output of received serial commands
	const static byte DB_STEPS		= B00000010;	// Debug flag -- toggles output of motor step information 
	const static byte DB_MOTOR		= B00000100;	// Debug flag -- toggles output of general motor information 
	const static byte DB_GEN_SER	= B00001000;	// Debug flag -- toggles output of responses to certain serial commands
	const static byte DB_FUNCT		= B00010000;	// Debug flag -- toggles output of debug messages within most functions
	const static byte DB_CONFIRM	= B00100000;	// Debug flag -- toggles output of success and failure messages in response to serial commands

	void setState(byte state);
	byte getState();

	template <typename T>
	void ser(T data){
		if (usb_debug & DB_GEN_SER)
			USBSerial.print(data);
	}

	template <typename T>
	void serln(T data){
		if (usb_debug & DB_GEN_SER)
			USBSerial.println(data);
	}

	template <typename T>
	void funct(T data){
		if (usb_debug & DB_FUNCT)
			USBSerial.print(data);
	}

	template <typename T>
	void functln(T data){
		if (usb_debug & DB_FUNCT)
			USBSerial.println(data);
	}

	template <typename T>
	void com(T data){
		if (usb_debug & DB_COM_OUT)
			USBSerial.print(data);
	}

	template <typename T>
	void comln(T data){
		if (usb_debug & DB_COM_OUT)
			USBSerial.println(data);
	}

	template <typename T>
	void com(T data, int base){
		if (usb_debug & DB_COM_OUT)
			USBSerial.print(data);
	}

	template <typename T>
	void comln(T data, int base){
		if (usb_debug & DB_COM_OUT)
			USBSerial.println(data, base);
	}

	template <typename T>
	void confirm(T data){
		if (usb_debug & DB_CONFIRM)
			USBSerial.print(data);
	}

	template <typename T>
	void confirmln(T data){
		if (usb_debug & DB_CONFIRM)
			USBSerial.println(data);
	}

	template <typename T>
	void steps(T data){
		if (usb_debug & DB_STEPS)
			USBSerial.print(data);
	}

	template <typename T>
	void stepsln(T data){
		if (usb_debug & DB_STEPS)
			USBSerial.println(data);
	}
};

extern DebugClass Debug;

#endif

