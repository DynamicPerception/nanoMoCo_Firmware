/*


OpenMoco

Time-lapse Core Engine

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


/* 

 *******************************
 Mapping of Data Positions in EEPROM memory
 *******************************

 (position count starts at zero)
 
 flash enabled   = 0
 dev_addr        = 1
 
 
*/




boolean eeprom_saved() {
  
    // read eeprom saved status
    
  byte saved = EEPROM.read(0);
 
   // EEPROM memory is by default set to 1, so we
   // set it to zero if we've written data to eeprom
  return( ! saved );
}

void eeprom_saved( boolean saved ) {
  // set eeprom saved status

   // EEPROM memory is by default set to 1, so we
   // set it to zero if we've written data to eeprom
  
  EEPROM.write(0, !saved);
}



// One can ask why I didn't use the templates from http://www.arduino.cc/playground/Code/EEPROMWriteAnything
// The primary reason here is that we're going to be calling these functions OFTEN, and I _really_ don't 
// want the templates getting inlined _everywhere_, what a mess!  So, rather than be slick, let's just declare
// what we mean, and do it once - forget about the overhead of the function call, and worry more about
// flash and stack abuse 


void eeprom_write( int pos, byte& val, byte len ) {
  byte* p = (byte*)(void*)&val;
  for( byte i = 0; i < len; i++ )
    EEPROM.write(pos++, *p++);    

    // indicate that memory has been saved
  eeprom_saved(true);

}
    
void eeprom_write( int pos, unsigned int& val ) {
  byte* p = (byte*)(void*)&val;   
  eeprom_write(pos, *p, sizeof(int));  
}

void eeprom_write( int pos, unsigned long& val ) {
  byte* p = (byte*)(void*)&val;   
  eeprom_write(pos, *p, sizeof(long));    
}

void eeprom_write( int pos, float& val ) {
  byte* p = (byte*)(void*)&val;   
  eeprom_write(pos, *p, sizeof(float));    
}

void eeprom_write( int pos, byte& val ) {  
  EEPROM.write(pos, val);
    // indicate that memory has been saved
  eeprom_saved(true);
}





 // read functions

void eeprom_read( int pos, byte& val, byte len ) {
  byte* p = (byte*)(void*)&val;
  for(byte i = 0; i < len; i++) 
    *p++ = EEPROM.read(pos++);
}

void eeprom_read( int pos, byte& val ) {
  val = EEPROM.read(pos);
}


void eeprom_read( int pos, int& val ) {
  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(int));
}

void eeprom_read( int pos, unsigned int& val ) {

  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(int));
  
}

void eeprom_read( int pos, unsigned long& val ) {

  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(long));
  
}

void eeprom_read( int pos, float& val ) {

  byte* p = (byte*)(void*)&val;
  eeprom_read(pos, *p, sizeof(float));
  
}
    
void write_all_eeprom_memory() {

    // write default values into eeprom

  eeprom_write(1, device_address);
  
      
}


 // restore memory
 
void restore_eeprom_memory() {

    // read eeprom stored values back into RAM
    
    eeprom_read(1, device_address);
    
}

