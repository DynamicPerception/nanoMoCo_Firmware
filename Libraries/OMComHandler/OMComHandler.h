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

#ifndef	OM_COMHANDLER_H
#define OM_COMHANDLER_H


#include <inttypes.h>

 // pins for COM lines on nanoMoCo Board
 
#define OM_COM1 2
#define OM_COM2 3
#define OM_COM3 19


/**

  @brief
  Common Line Handler
  
  The Common Line Handler class allows for handling of the three common
  lines designed for triggering different behaviors between nodes, and 
  for synchronizing timing between a 'master'-timing node and 'slave' nodes
  in a network of Motion Axis devices.
  
  This class handles the required functionalitty for both the slave and the
  master node.
  
  There must only be one instance of this class in existance at a time, you
  cannot use two instances of this class.
  
  
  Please note that OMComHandler uses Arduino pins 2, 3, and 19 (analog 5) - it 
  is NOT possible to change this!
  
  There are three common lines available to the MoCoBus: Common 1 (2), Common 2 (3), and
  Common 3 (19).  Common 3 is used for the timing signal between nodes, and is implemented
  in this library using a pin change interrupt.  If Common 1 and 2 are monitored, a hardware
  interrupt is used for these pins.
 
  Master Node Example:
  
   @code
  
  // Master Node
  
  
  OMComHandler ComMgr;
  
  void setup() {
    ComMgr = OMComHandler();
    	// configure line handler as master
    ComMgr.master(true);
  }
  
  void loop() {
  
    // do something
    
    ..
    
    // trigger a complete signal
    ComMgr.masterSignal();
    
  }
  @endcode
  
  Slave Node Example:
  
  @code
  
  // Slave Node
  
  
  OMComHandler ComMgr;
  
  void setup() {
    ComMgr = OMComHandler();
    	// configure line handler as slave
    ComMgr.master(false);
  }
  
  void loop() {

    // received a complete signal
    if( ComMgr.slaveClear() == true ) {
  
      // do something
    
      ..
    }
    
    
  }
  @endcode
  
 Additionally, it is possible to monitor either Common 1 or Common 2 for a signal.  In this
 case, a callback handler is required, and this callback handler is called when a signal is
 brought LOW and then back to HIGH, and is passed an unsigned integer with the number of microseconds
 the pin was brought low.
 
 For example:
 
 @code
 
 OMComHandler ComMgr;
 
 volatile unsigned int trip_time = 0;
 
 void setup() {
    ComMgr = OMComHandler();
        // watch COM1
    ComMgr.watch(1);
    ComMgr.watchHandler(tripped);
 }
 
 void loop() {
 
 
    if( trip_time > 0 ) {
        
        if( trip_time > 100 ) {
            digitalWrite(13, HIGH);
            delay(2000);
            digitalWrite(13, LOW);
        }
        else {
            digitalWrite(13, HIGH);
            delay(1000);
            digitalWrite(13, LOW);
        }
    
        trip_time = 0;
    }
  
 
 }
 
 void tripped(unsigned int p_us) {
    trip_time = p_us;
 }
 @endcode
 
  @author C. A. Church

  (c) 2011-2012 C. A. Church / Dynamic Perception
  
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
  
class OMComHandler
{
	
private:
	
	bool m_isMaster;

    static bool m_which;
    volatile static unsigned long m_isrUs;
    
	void _masterFollow();
	void _masterLead();
    
    static void _isrFire();
    
    static void(*f_isrCB)(uint16_t);
	
public:
	
	OMComHandler(); // constructor
	
	bool master();
	void master(bool p_mast);
	void masterSignal();
	static bool slaveClear();
    
    static void watch(uint8_t p_which);
    static void watchHandler(void(*p_func)(uint16_t));
    static void stopWatch();
	
	
};


#endif
