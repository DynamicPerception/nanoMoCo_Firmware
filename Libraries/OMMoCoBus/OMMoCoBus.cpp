
#include "OMMoCoBus.h"
#include "Arduino.h"

// #include "OMUtils.h"

/*


OpenMoco Core Libraries

Author: C.A. Church

(c) 2011 Dynamic Perception LLC

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


/*

  ========================================
  MoCoBus Core Library
  ========================================

*/


#include <util/delay.h>


/** Constructor

 Constructs a new instance of the class. Accepts a hardware serial object,
 the digital pin # of the transceiver DE/RE pin combination, and the
 current device address.  You may change the address later using address()

 Note: for masters, the address will always be 0, and for slaves the address
 must be greater than 1.

 @param c_serObj
 An Arduino HardwareSerial object

 @param c_dePin
 The transceiver DE/RE digital I/O pin number

 @param c_dAddr
 The device address

 */

OMMoCoBus::OMMoCoBus(HardwareSerial& c_serObj, uint8_t c_dePin, unsigned int c_dAddr) {

  m_serObj = &c_serObj;
  m_dePin = c_dePin;
  m_devAddr = c_dAddr;
  m_bufSize = 0;
  f_newAddr = 0;

  pinMode(m_dePin, OUTPUT);
  digitalWrite(m_dePin, LOW);

}


/** Get Address

 Gets the current address assigned to this node.

 @return
 Device address
 */

unsigned int OMMoCoBus::address() {
  return(m_devAddr);
}

/** Set Address

 Sets the current address assigned to this device

 @param p_addr
 Device address
 */

void OMMoCoBus::address(unsigned int p_addr) {
  m_devAddr = p_addr;

  	// an address change occurred, call address change
  	// callback if defined

  if( f_newAddr != 0 )
  	  f_newAddr(m_devAddr);

}

/** Set Address Change Callback

 This method sets the callback which will be called whenever an address
 change occurs.  This is highly useful for devices that wish to save
 their address to EEPROM so that it remains fixed between power cycles,
 especially when the address change command is received as part of the
 core MoCoBus protocol.

 The callback function pointer passed must accept a single argument
 of a single byte, which is the new address assigned to the device,
 and return void.

 @param p_Func
 A function pointer matching the template void function(byte);
*/

void OMMoCoBus::addressCallback(void(*p_Func)(uint8_t)) {
	f_newAddr = p_Func;
}

/** Get A Packet

  Gets a complete packet off of the bus, if available.

  If there is a complete packet received over the bus, and if it is destined
  for this device, this method will place the packet data into the buffer
  and return the packet code.

  Note: this method is not intended for direct use in Nodes, instead see
  OMMoCoNode::check() which is the correct way to check for a received
  command packet.

  @return
  Packet code, or 0 if not intended for us/error packet

  */

uint8_t OMMoCoBus::getPacket() {

 uint8_t command_val    = 0;
 uint8_t com_byte_count = 0;
 //uint8_t dat            = 0;
 uint8_t ret            = 0;
 
 m_isBCast = false;

 bool not_us = false;

     // we check serial.available() here even
     // though this->_getNextByte() does, because
     // a timeout is applied waiting for next command
     // in this->_getNextByte() - since this is called
     // every cycle of the main loop, we don't want
     // to introduce a delay unless we know of a
     // byte waiting to be read

 int avail = m_serObj->available();

      // serial does not have data in the buffer
 if( avail <= 0 )
 	 return(0);

 	 // is this packet intended for us?
 uint8_t stat = this->_targetUs();

 if(stat == OM_SER_ERR || stat == OM_SER_TIMEOUT) {
       // got an error? timeout error, really.
  //   this->_flushSerial();
     return(0);
 }
 else if( stat == OM_SER_IS_BCAST ) {
 	 // command was a broadcast command
 	 m_isBCast = true;
 }
 else if( stat != OM_SER_OK ) {
       // command was for someone else
       // (but we need to read the rest of the data
       // off the line to make sure we don't keep part of it
       // in the buffer)
     not_us = true;
 }

    // get command byte
 ret = this->_getNextByte(command_val);

 if ( ret != OM_SER_OK ) {
       // well, this is just embarassing
  //    this->_flushSerial();
      return(0);
 }

 if( command_val > 0 ) {
       // get data length (even if not our own command)
     ret = this->_getNextByte(com_byte_count);

     if ( ret != OM_SER_OK ) {
         // well, this is just embarassing
  //     this->_flushSerial();
       return(0);
     }
 }

    // check for overflow
    // note: we will still be in error condition, as
    // there will be values left in the serial buffer
    // on the next read, from this command - so don't
    // exceed it, eh?  This is just to prevent memory
    // corruption.  It is not safe otherwise

 if(com_byte_count > OM_SER_BUFLEN)
   com_byte_count = OM_SER_BUFLEN;


   // clear out any previous command data
 memset(m_serBuffer, 0, sizeof(uint8_t) * OM_SER_BUFLEN);
 m_bufSize = 0;


   // populate command data buffer
 for( int i = 1; i <= com_byte_count; i++ ) {
     // get com_byte_count character values from the serial
     // buffer

     uint8_t dat = 0;
     uint8_t ret = this->_getNextByte(dat);

     if( ret != OM_SER_OK )
       return(0);

     m_serBuffer[i - 1] = dat;
     m_bufSize++;
 }

 if( not_us == true ) {
 	// the command was not intended for us
 	 memset(m_serBuffer, 0, sizeof(uint8_t) * OM_SER_BUFLEN);
 	 return(0);
 }


 return( command_val );
}


/** Send Packet Header

 Sends a complete packet header, given an address to target, a packet code to
 ssend, and the length of data that will be sent.

 This method is part of a low-level toolkit for creating MoCoBus handlers, and
 should not be used in part of a normal Node implementation.  Instead, see
 OMMoCoNode::response().
 
 If you send a packet header to the broadcast address (as defined by OM_SER_BCAST_ADDR),
 the method isBroadcast() will return true immediately after calling this method. You
 can use this in higher-level classes to determine if a response should be expected.

 A MoCoBus packet always starts with a 10-byte header with the following structure:

 <table border=1 width="80%">
 <tr>
 	<td>0</td>
 	<td>1</td>
 	<td>2</td>
 	<td>3</td>
 	<td>4</td>
 	<td>5</td>
 	<td>6</td>
 	<td>7</td>
 	<td>8</td>
 	<td>9</td>
 </tr>
 <tr>
 	<td colspan=6><b>Break Sequence</b></td>
 	<td colspan=2><b>Address</b></td>
 	<td><b>Packet Code</b></td>
 	<td><b>Data Len</b></td>
 </tr>
 <tr>
 	<td>0</td>
 	<td>0</td>
 	<td>0</td>
 	<td>0</td>
 	<td>0</td>
 	<td>255</td>
 	<td colspan=2>0-65535</td>
	<td>0-255</td>
	<td>0-255</td>
 </tr>
 </table>

 Note that all multi-byte values sent using the MoCoBus specification are in
 Big-Endian (Network) byte order!

 There are two types of packets which may be sent on MoCoBus, and they are not
 differentiated except in the order in which they occur:

 Command Packet

 A command packet is sent by a Master, and may have up to 254 packet codes, from
 1-255.  0 is never a valid packet code for a command packet.  Up to 255 data
 bytes may be included with the packet, as long as they do not include a sequence
 which corresponds to the break sequence noted above.

 Generally speaking, due to the limitation of 254 packet codes, the protocol for
 a specific node type will likely define a series of sub-codes for use as the
 first byte or set of bytes in the packet data.


 Response Packet

 A response packet is sent by a Node, in response to a command packet, back to
 the master.  It should only have two valid packet codes: 0 and 1, with 0 indicating
 an error occured processing the command and 1 indicating that the command
 was successfully completed.  Response packets may or may not contain actual
 packet data, this will depend on the specific implementation by the node.

 
 @param p_addr
 The device address to ssend the packet header to

 @param p_code
 The packet code to use

 @param p_dlen
 The packet data length, in bytes

 */

void OMMoCoBus::sendPacketHeader(unsigned int p_addr, uint8_t p_code, uint8_t p_dlen) {
	
	if( p_addr == OM_SER_BCAST_ADDR )
		m_isBCast = true;
	else
		m_isBCast = false;
	
		// start sequence of five nulls
	for( uint8_t i = 0; i <= 4; i++ ) {
		this->write((uint8_t) 0);
	}

		// start sequence termination
	this->write((uint8_t) 255);

		// target address
	this->write(p_addr);

		// command code or response code
	this->write(p_code);

		// data length
	this->write(p_dlen);
}

/** Received Packet was Broadcast

  Call after using getPacket() to determine if the packet received was a 
  broadcast packet.
  
  For example:
  
  @code
  
  uint8_t ret = getPacket();
  
  if( ret != 0 ) {
  	if( isBroadcast() == true ) {
  		// do something with the broadcast packet
  	}
  	else {
  		// directed packet
  	}
  }
  @endcode
  
  @return
  True if the previously received packet was a broadcast packet, false if
  directed to the specific device.
  */
  
bool OMMoCoBus::isBroadcast() {

	return( m_isBCast );
}

/** Write Data to Bus
 
 Writes packet data to the bus, should only ever be used after sendPacketHeader().
 
 */

void OMMoCoBus::write(uint8_t p_dat) {
    
    // TODO: Abstract for non Atmega328P platforms
    
    while (!(UCSR0A & _BV(UDRE0)));
    
	// set xmit pin high
    PORTD |= _BV(PORTD5);  
    // clear tx complete flag before write, just incase serial object doesn't
    UCSR0A |= _BV(TXC0);
    
    UDR0 = p_dat;		
    
	
    while(!(UCSR0A & _BV(TXC0)) );
    
    _delay_us(OM_SER_CLEAR_TM);
    
    PORTD &= ~_BV(PORTD5);
    
    
}

/** Write Data to Bus

 Writes packet data to the bus, should only ever be used after sendPacketHeader().

*/

void OMMoCoBus::write( unsigned int p_dat ) {

    write( (uint8_t) (p_dat >> 8) );
    write( (uint8_t) p_dat);

}

/** Write Data to Bus

 Writes packet data to the bus, should only ever be used after sendPacketHeader().

*/

void OMMoCoBus::write( int p_dat ) {

    write( (uint8_t) (p_dat >> 8) );
    write( (uint8_t) p_dat);

}

/** Write Data to Bus

 Writes packet data to the bus, should only ever be used after sendPacketHeader().

*/

void OMMoCoBus::write( unsigned long p_dat ) {

    write( (uint8_t) (p_dat >> 24) );
    write( (uint8_t) (p_dat >> 16) );
    write( (uint8_t) (p_dat >> 8) );
    write( (uint8_t) p_dat);

}

/** Write Data to Bus

 Writes packet data to the bus, should only ever be used after sendPacketHeader().

*/

void OMMoCoBus::write( long p_dat ) {

    write( (uint8_t) (p_dat >> 24) );
    write( (uint8_t) (p_dat >> 16) );
    write( (uint8_t) (p_dat >> 8) );
    write( (uint8_t) p_dat);

}


/** Retrieve Packet Data Buffer

 Retrieves the complete packet data buffer (without the packet header) after a
 packet has been received from the bus.

 WARNING: This method returns a pointer to the -actual- buffer. Do not store
 this pointer between packets or attempt to modify it directly.

 Check OM_SER_BUFLEN constant for maximum buffer length.

 @return
 Pointer to packet data buffer
 */

uint8_t* OMMoCoBus::buffer() {
   return(m_serBuffer);
}

/** Retrieve Buffer Length

 Retrieves the length of data, in bytes, currently stored in the
 packet data buffer.

 @return
 The length of data, in bytes, in the packet data buffer
 */

uint8_t OMMoCoBus::bufferLen() {
  return(m_bufSize);
}


uint8_t OMMoCoBus::_targetUs() {

   uint8_t thsChar = 1;
   uint8_t ret = this->_getNextByte(thsChar);

     // timeout? other error?
   if( ret != OM_SER_OK )
     return(OM_SER_TIMEOUT);

     // if character is null...
   if( thsChar == 0 ) {
       // until we get enough nulls for a start command
       // or read something that isn't null...

     uint8_t nullCount = 0;

     while( nullCount < 5 && thsChar == 0 ) {
	nullCount++;

	 // null count met
	if( nullCount == 5 )
		break;

	ret = this->_getNextByte(thsChar);

	if( ret != OM_SER_OK )
		return(OM_SER_TIMEOUT);

     }

       // did not receive a proper start sequence
     if( nullCount < 5 )
       return(OM_SER_ERR);

     thsChar = 0;

     ret = this->_getNextByte(thsChar);

     if( ret != OM_SER_OK )
     	     return(OM_SER_TIMEOUT);

     if( thsChar != 255 )
     	     return(OM_SER_ERR);

       // get target address

     unsigned int addr = 0;
     ret = this->_getNextByte(thsChar);

     if( ret != OM_SER_OK )
     	     return(OM_SER_TIMEOUT);

     addr = (unsigned int) thsChar << 8;

     ret = this->_getNextByte(thsChar);

     if( ret != OM_SER_OK )
     	     return(OM_SER_TIMEOUT);

     addr |= (unsigned int) thsChar;

       // is not destined for us?
   
       if( addr == OM_SER_BCAST_ADDR )
       	     return(OM_SER_IS_BCAST);   	       
       else if( addr != m_devAddr )
     	     return(OM_SER_NOT_US);

   } // end if(thsChar == 0
   else {
       // uint8_t invalid
       return(OM_SER_ERR);
   }

  return(OM_SER_OK);

}

uint8_t OMMoCoBus::_getNextByte(uint8_t& p_data) {

  uint8_t ret = OM_SER_OK;
  uint8_t* p = (uint8_t*)(void*) &p_data;

  if( m_serObj->available() > 0 ) {
    *p = (uint8_t) m_serObj->read();
  }
  else {
    unsigned long timer = millis();
       // wait until we have data ready
    while( m_serObj->available() < 1 ) {
       if( millis() - timer > OM_SER_WAIT ) {
         // timed out waiting for complete input
         return( OM_SER_TIMEOUT );
       }
    }
    *p = (uint8_t) m_serObj->read();
  }


  return(ret);
}

void OMMoCoBus::_flushSerial() {
 while( m_serObj->available() > 0 ) {
    m_serObj->flush();
  }
}

/** Convert Network-order Bytes to Integer
 
 @param p_dat
 A pointer to an array of bytes
 
 @return 
 A signed integer
 */

int OMMoCoBus::ntoi(uint8_t* p_dat) {
	int ret = ((int) p_dat[0] << 8) + (int) p_dat[1];
	return(ret);
}

/** Convert Network-order Bytes to Unsigned Integer
 
 @param p_dat
 A pointer to an array of bytes
 
 @return 
 An unsigned integer
 */

unsigned int OMMoCoBus::ntoui(uint8_t* p_dat) {
	unsigned int ret = ((unsigned int) p_dat[0] << 8) + (unsigned int) p_dat[1];
	return(ret);
}

/** Convert Network-order Bytes to Unsigned Long
 
 @param p_dat
 A pointer to an array of bytes
 
 @return 
 An unsigned long
 */

unsigned long OMMoCoBus::ntoul(uint8_t* p_dat) {
	unsigned long ret  = (unsigned long) ( ((unsigned long) p_dat[0] << 24) + (unsigned long) p_dat[1] ) << 16;
    ret |= (unsigned long) ( ( (unsigned long) p_dat[2] << 8 ) + ( (unsigned long) p_dat[3] ) );
	return(ret);
}

/** Convert Network-order Bytes to Long
 
 @param p_dat
 A pointer to an array of bytes
 
 @return 
 A long
 */

long OMMoCoBus::ntol(uint8_t* p_dat) {
	unsigned long ret  = (long) ( ((long) p_dat[0] << 24) + (long) p_dat[1] ) << 16;
    ret |= (long) ( ( (long) p_dat[2] << 8 ) + ( (long) p_dat[3] ) );
	return(ret);
}

/** Convert Network-order Bytes to Float
 
 @param p_dat
 A pointer to an array of bytes
 
 @return 
 A float
 */

float OMMoCoBus::ntof(uint8_t* p_dat) {
	float ret;
	unsigned long * _fl = (unsigned long *) (&ret);
	*_fl  = (unsigned long) ( ((unsigned long) p_dat[0] << 24) + (unsigned long) p_dat[1] ) << 16;
	*_fl |= (unsigned long) ( ( (unsigned long) p_dat[2] << 8 ) + ( (unsigned long) p_dat[3] ) );
	return(ret);
}



