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


// These defines are used for the Limit Switch Pin Change Registers

// // for port PCINT32/PD7/AIN1
//#define LIMIT_ENABLE  PCIE2
//#define LIMIT_MASK    PCMSK2
//#define LIMIT_INT     PCINT23
//#define LIMIT_VECT    PCINT2_vect
//#define LIMIT_PREG    PIND
//#define LIMIT_PIN     PIND7
//
//
//volatile byte ls_pc_hist = 0xFF;
//
//void limitSwitch(bool p_enable) {
//
//    // limit switch is hard-defined to PBT_PIN, which is digital 7, or PCINT23
//    
//    if( p_enable ) {
//      
//      attachInterrupt(LIMIT_PIN, ISR(), CHANGE);
//      
//      PCICR |= (1 << LIMIT_ENABLE);
//      LIMIT_MASK |= (1 << LIMIT_INT);
//    }
//    else {
//      PCICR ^= (1 << LIMIT_ENABLE);
//      LIMIT_MASK ^= (1 << LIMIT_INT);
//    }
//      
//  
//}
//
//
// // Our ISR for the pin change
// 
//ISR(LIMIT_VECT) {
//
//    // if we've just switched low (pin was previouslty high, and our current state is LOW)
//  if( ! ( LIMIT_PREG & (1 << LIMIT_PIN))  && (ls_pc_hist & (1 << LIMIT_PIN) ) ) {
//    force_stop = true;
//    digitalWrite(DEBUG_PIN, HIGH);
//  }
//    
//  ls_pc_hist = LIMIT_PREG;
//    
//}

