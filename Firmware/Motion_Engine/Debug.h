// Debug.h

#ifndef _DEBUG_h
#define _DEBUG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "OMMoCoPrint.h"

class DebugClass
{
 private:
	 void init();
	 byte m_debug_flag;	// Byte holding debug output flags
	 bool m_usb;
	 bool m_moco;
	 OMMoCoPrintClass *m_mocoPrint;

 public:
	DebugClass(OMMoCoPrintClass *c_mocoPrint);		
	const static byte DB_COM_OUT	= B00000001;	// Debug flag -- toggles output of received serial commands
	const static byte DB_STEPS		= B00000010;	// Debug flag -- toggles output of motor step information 
	const static byte DB_MOTOR		= B00000100;	// Debug flag -- toggles output of general motor information 
	const static byte DB_GEN_SER	= B00001000;	// Debug flag -- toggles output of responses to certain serial commands
	const static byte DB_FUNCT		= B00010000;	// Debug flag -- toggles output of debug messages within most functions
	const static byte DB_CONFIRM	= B00100000;	// Debug flag -- toggles output of success and failure messages in response to serial commands

	void setState(byte state);		
	byte getState();

	bool setUSB(bool enabled);
	bool getUSB();

	bool setMoco(bool enabled);
	bool getMoco();

	template <typename T>
	void ser(T data){
		if (m_debug_flag & DB_GEN_SER){
			if (m_usb)
				USBSerial.print(data);
			if (m_moco)
				m_mocoPrint->print(data);
		}
	}

	template <typename T>
	void serln(T data){
		if (m_debug_flag & DB_GEN_SER){
			if (m_usb)
				USBSerial.println(data);
			if (m_moco)
				m_mocoPrint->println(data);
		}
	}

	template <typename T>
	void funct(T data){
		if (m_debug_flag & DB_FUNCT){
			if (m_usb)
				USBSerial.print(data);
			if (m_moco)
				m_mocoPrint->print(data);
		}
	}

	template <typename T>
	void functln(T data){
		if (m_debug_flag & DB_FUNCT){
			if (m_usb)
				USBSerial.println(data);
			if (m_moco)
				m_mocoPrint->println(data);
		}
	}

	template <typename T>
	void com(T data){
		if (m_debug_flag & DB_COM_OUT){
			if (m_usb)
				USBSerial.print(data);
			if (m_moco)
				m_mocoPrint->print(data);
		}
	}

	template <typename T>
	void comln(T data){
		if (m_debug_flag & DB_COM_OUT){
			if (m_usb)
				USBSerial.println(data);
			if (m_moco)
				m_mocoPrint->println(data);
		}
	}

	template <typename T>
	void com(T data, int base){
		if (m_debug_flag & DB_COM_OUT){
			if (m_usb)
				USBSerial.print(data, base);
			if (m_moco)
				m_mocoPrint->print(data, base);
		}
	}

	template <typename T>
	void comln(T data, int base){
		if (m_debug_flag & DB_COM_OUT){
			if (m_usb)
				USBSerial.println(data, base);
			if (m_moco)
				m_mocoPrint->println(data, base);
		}
	}

	template <typename T>
	void confirm(T data){
		if (m_debug_flag & DB_CONFIRM){
			if (m_usb)
				USBSerial.print(data);
			if (m_moco)
				m_mocoPrint->print(data);
		}
	}

	template <typename T>
	void confirmln(T data){
		if (m_debug_flag & DB_CONFIRM){
			if (m_usb)
				USBSerial.println(data);
			if (m_moco)
				m_mocoPrint->println(data);
		}
	}

	template <typename T>
	void steps(T data){
		if (m_debug_flag & DB_STEPS){
			if (m_usb)
				USBSerial.print(data);
			if (m_moco)
				m_mocoPrint->print(data);
		}
	}

	template <typename T>
	void stepsln(T data){
		if (m_debug_flag & DB_STEPS){
			if (m_usb)
				USBSerial.println(data);
			if (m_moco)
				m_mocoPrint->println(data);
		}
	}
};
#endif

