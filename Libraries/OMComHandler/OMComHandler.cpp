/*

Common Line Handler Library

OpenMoco nanoMoCo Core Engine Libraries 

See www.openmoco.org for more information

(c) 2008-2011 C.A. Church / Dynamic Perception LLC

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

#include "Arduino.h"
#include "OMComHandler.h"

bool OMComHandler::s_isTripped = false;

/** Class Constructor

  Creates a new instance of the class.  You may not create more
  than one of this class in any program.
  
  */
  
OMComHandler::OMComHandler() {
	m_isMaster = false;
	s_isTripped = false;
}

void OMComHandler::_masterFollow() {
      // slave, attach an interrupt to the
      // common line, engage pull-up resistor
    pinMode(OM_COM1, INPUT);
    digitalWrite(OM_COM1, HIGH);
    attachInterrupt(0, _slaveTripped, FALLING);
}

void OMComHandler::_masterLead() {
      // master, bring the common line high (bring low to signal slaves)
    detachInterrupt(0);
    pinMode(OM_COM1, OUTPUT);
    digitalWrite(OM_COM1, HIGH);
}	


void OMComHandler::_slaveTripped() {
	s_isTripped = true;
}

/** Get Master Parameter

  Returns the current master parameter value.
  
  @return bool
  true if the instance is a master timing node, false if it is a slave
  
  */
  

bool OMComHandler::master() {
	return(this->m_isMaster);
}

/** Set Master Parameter

  Sets the current master parameter value.
  
  @param mast
  true if the instance is a master timing node, false if it is a slave
  
  */

void OMComHandler::master(bool mast) {
	this->m_isMaster = mast;
	
	if( this->m_isMaster) {
		this->_masterLead();
	}
	else {
		this->_masterFollow();
	}
	
}

/** Send Signal as Master

  Triggers the timing signal line (COM1), by bringing it LOW and
  signalling all slaves monitoring COM1. The line is held LOW for
  5mS.
  
  Note: if the instance is not set as master, this method will do
  nothing.
  
  Example:
  @code
  	ComMgr.masterSignal();
  	Serial.println("Sent Signal as Master!");
  @endcode	

  */

void OMComHandler::masterSignal() {
	if( this->m_isMaster ) {
	      // we're the master, send a 5mS low pulse on
	      // common line
	    digitalWrite(OM_COM1, LOW);
	    delay(5);
	    digitalWrite(OM_COM1, HIGH);
    	}
}

/** Slave Clear to Go

  When the instance of the class is a slave, calling this method will 
  indicate whether or not a master signal was received since the last 
  time this method was called.
 
  The master signal flag is always reset to false after calling this method,
  and will not count how many times its triggered between calls to this method.

  Example:
  @code
  if( ComMgr.master() == false ) {
    if( ComMgr.slaveClear() == true ) {
    	Serial.println("Received Signal From Master!");
    }
  }
  @endcode
  
  @return
  true if signal was received, false if not
  
  */
  
bool OMComHandler::slaveClear() {
	bool ret = s_isTripped;
	s_isTripped = false;
	return(ret);
}

