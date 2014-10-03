/*


Motion Engine

See dynamicperception.com for more information


(c) 2008-2012 C.A. Church / Dynamic Perception LLC

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
  EEPROM write/read functions
  ========================================
  
*/




// EEPROM Memory Layout Version, change this any time you modify what is stored
const unsigned int MEMORY_VERSION = 2;




/** Check EEPROM Status

 If EEPROM hasn't been stored, or EEPROM version does not
 match our version, it saves our variables to eeprom memory.
 
 Otherwise, it reads stored variables from EEPROM memory
 
 @author C. A. Church
 */
 
void eepromCheck() {
  
  using namespace OMEEPROM;
    
  if( saved() ) {
      if( version() != MEMORY_VERSION )
        eepromWrite();
      else
        eepromRestore();
  }
  else {
    eepromWrite();
  }
    
}

 /** Write All Variables to EEPROM */
 
void eepromWrite() {
  using namespace OMEEPROM;
 
  version(MEMORY_VERSION);
  
  write(EE_ADDR, device_address);
  write(EE_NAME, *device_name, 16);

  write(EE_POS_0, motor[0].currentPos());
  write(EE_END_0, motor[0].endPos());
  write(EE_START_0, motor[0].startPos());
  write(EE_STOP_0, motor[0].stopPos());
  
  write(EE_POS_1, motor[1].currentPos());
  write(EE_END_1, motor[1].endPos());
  write(EE_START_1, motor[1].startPos());
  write(EE_STOP_1, motor[1].stopPos());
  
  write(EE_POS_2, motor[2].currentPos());
  write(EE_END_2, motor[2].endPos());
  write(EE_START_2, motor[2].startPos());
  write(EE_STOP_2, motor[2].stopPos());

  
}


 /** Read all variables from EEPROM */
 
void eepromRestore() {
  using namespace OMEEPROM;
  
  read(EE_ADDR, device_address);
  read(EE_NAME, *device_name, 16);

  read(EE_POS_0, motor[0].currentPos());
  read(EE_END_0, motor[0].endPos());
  read(EE_START_0, motor[0].startPos());
  read(EE_STOP_0, motor[0].stopPos());

  read(EE_POS_1, motor[1].currentPos());
  read(EE_END_1, motor[1].endPos());
  read(EE_START_1, motor[1].startPos());
  read(EE_STOP_1, motor[1].stopPos());

  read(EE_POS_2, motor[2].currentPos());
  read(EE_END_2, motor[2].endPos());
  read(EE_START_2, motor[2].startPos());
  read(EE_STOP_2, motor[2].stopPos());
  
}


