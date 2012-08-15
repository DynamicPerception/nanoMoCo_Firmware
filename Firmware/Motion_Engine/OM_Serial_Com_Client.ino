/*


OpenMoco

Time-lapse Core Engine

See www.openmoco.org for more information

Author: C.A. Church

(c) 2008-2012 Dynamic Perception LLC

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

 =========================================
 MoCoBus Node Command Handling
 nanoMoCo Node Type
 =========================================
 
*/

/* 

  

   --> NOTE: The serial protocol is a BINARY protocol, not an ascii
   protocol.  All data transmitted should be the binary representation.
   
   --> All multi-byte values shall be transmitted in BIG ENDIAN form,
   that is, always start with the MSB and end with the LSB
   
   
   
   MoCoBus Packet Data Definition for nanoMoCo Node:   
   
   Data Section:
   
   Byte0   = Command code (1-254)
   Byte1   = Command data length (0-254 bytes)
   Byte2-x = <data length bytes>
   
   
   --> For each request, a response will be sent indicating success or failure.
   A successful request will respond with a binary value of 1, 0x01.  A 
   failed request will result in a response of 0, 0x00.
   
   See the further below for complete details of the response values.
   
   
   Command codes:
   
   1: program control
   
      Data length: 1-4
      
      Data Byte 0 value:

      0 (noop/test)

        Does nothing.  Responds back ok.
        Used for testing if the device is listening
        
      1 (start)
      2 (pause)
      3 (stop)
      4 (camera on)
      5 (camera off)
      6 (move motor now)
      
        Byte 1    : dir (0/1)
        Bytes 2-3 : distance to move
        
      7 (send motor home)
      
      8 (shoot camera now)

        Bytes 1-4  : exposure time
 
      9 (motor kill line)
      
        Byte 1 : enable/disable (1/0)
        
          -- triggers motor sleep line high 
             between motor moves

      10 (motor maximum step rate)

        Bytes 1-2 : steps/second
  
      11 (motor maximum speed)
        
        Bytes 1-2 : steps/second
        

      12 (defunct)

  
      13 (set exposure action mod)

        Bytes 1-2 : divisor

      14 (motor async enable)

        Byte 1: on/off
 
        
      15 (flash debug led)
      
        Byte 1 : on/off (1 = on);
        
      16 (stop motor now)
      
      17 (set motor microstep)
      
        Byte 1: ms divisor (1, 2, 4, 8, 16)
        
      18 (set timing master)
      
        Byte 1: on/off ( 1 = on )
        
      100 (get current status)
 
        Byte 1 : status type
   
        Values:
   
           0 (api version)
           
             returns current api version
             
             RESPONSE:
             
               1 byte:
               
                 version
                 
                 
           1 (program run status)
      
              indicates started, stopped, paused
  
              RESPONSE:
              
                1 byte:
                
                  0 (stopped or paused)
                  1 (running)
                  
              
           2 (program run time)
             
              indicates currenrt run time counter
              
              RESPONSE: 
              
                  4 bytes:
                  
                    current run time (unsigned long - 32 bits)
              
           3 (camera enabled status)
      
              indicates camera currently enabled/disabled status
              
              RESPONSE:
              
                  1 byte:
                  
                    0: camera off
                    1: camera on
                    
              
           4 (camera current shot count)
           
              indicates current camera shot count
              
              RESPONSE:
              
                  4  bytes:
                  
                    current shot count (unsigned long)
                    
           5 (camera interval time)
           
              indicates current camera interval time
              
              RESPONSE:
              
                  2 bytes:
                  
                    current interval time (unsigned int)
                    
           6 (camera exposure time)
           
              indicates current camera exposure time
              
              RESPONSE:
              
                  4 bytes:
                  
                    current exposure time (unsigned long)
                    
           7 (camera post time)
           
              indicates current camera post-exposure delay time
              
              RESPONSE:
              
                2 bytes:
                
                  current post delay time (unsigned int)
           
           8 (camera currently firing)
        
            
              indicates whether camera is currently exposing
              
              RESPONSE:
              
                  1 byte:
                  
                    0: camera not currently firing
                    1: camera currently firing
                    
           9 (motor enabled)
           
               indicates whether motor is enabled
               
               RESPONSE
               
                 1 byte:
                   
                    0: motor not enabled
                    1: motor enabled
                    
           10 (motor dir)
           
               indicates current motor direction
               
               RESPONSE:
               
                 1 byte:
                 
                   motor direction
                   
                   
           11 (motor steps moved)
           
               indicates total steps moved in program execution
               
               RESPONSE:
               
                 4 bytes:
                   
                     motor steps moved (unsigned long)
                     
           12 (motor dist from home)
          
                 NOTE: Returns signed value, +/-
               
               indicates current distance from home position
               
               RESPONSE:
               
                 4 bytes:
                 
                   distance from home position (signed long)
                   
           13 (noop - reserved for motor dist from end)
           
           14 (motor max step)
           
               indicates max step setting
               
               RESPONSE:
               
                 4 bytes:
                 
                   max steps (unsigned long)
               
           15 (motor ramp)
           
               indicates ramp value
              
               RESPONSE:
               
                 1 byte:
                   
                     motor ramp value
                     
           16 (motor backlash)
          
               indicates current backlash setting for motor
               
               RESPONSE:
               
                 1 byte:
                 
                   motor backlash value

           17 (motor steps)

              indicates steps between shots
   
              RESPONSE:
   
                 2 bytes:
      
                  motor steps value           
             

                  
           22 (timing master)
             
             indicates whether or not device is a timing master
             
             RESPONSE:
             
               1 byte:
               
                 ture/false
                 
                 
   2: set program data
   
     Data length: ...
     
     Byte 0: setting type
     
       1 (camera)
       2 (motor)
  

     Camera Controls (1) :
     
       Data Length: 3-5
       
        Byte 1: 
        
             1 (cycle delay)
             
               Bytes 2-5 : delay time
               
             2 (exposure time)
             
               Bytes 2-5 : exposure time
               
             3 (pre-focus tap time)
               
                Bytes 2-3 : pre-tap time
                
             4 (max shots)
             
               Bytes 2-3 : max shot count
              
             5 (camera post-delay)
               
               Bytes 2-3 : post delay time

             6 (focus high with shutter high)

               Byte 2  : focus w. shutter on/off (1/0)
              
             7 (camera cycle repeat count) 
             
               Byte 2  : count of repeat shots to make in cycle
                         ( 0 = off )
                  
               
      Motor Controls (2) : 
      
          Data Length: 2-6
          
          Byte 1: 
          
            1 (set motor steps)
            
              Bytes  2-3 : step count
              
            2 (defunct)
             
              Byte 2    : 
              
            3 (set motor dir)
            
              Byte 2    : direction
            
            4 (set motor max steps)  
            
              Byte 2-5  : max step count    

            5 (set motor enable)
            
              
            6 (set motor disable)
            
              
            7 (set current position as 'home')
            
              
            8 (set backlash amount in steps)
          
              Byte 2    : backlash steps  
            
            9 (continuous motion speed)
            
              Byte 2-3  : steps/second
            
            10 (easing algorithm)
            
              Byte 2   : type (1 = linear, 2 = quad, 3 = quadinv)
              
           11 (enable continuous motion)
           
             Byte 2 : on or off
              
              


            
 
  For all requests that set a value or trigger a command,
  (i.e. not status requests) the response data len will be
  0.  
  
  For all status requests, the response data len will be
  greater than or equal to 1.
  
  
         
       
*/
    

/* Handle normal commands

  2 = program control commands
  3 = data setting commands
 
  all else generates error
  */
  
void serCommandHandler(byte command, byte* buf) {

 switch(command) {   
   case 2:
           // program control
         serProgramAction(buf);
         break;

   case 3:
           // data setting
        serProgramData(buf);
        break;
        
   default :
           // anything else is an error
       Node.response(false);
       break;
 }

}

 // handle broadcast commands - do not send any
 // responses to a broadcast command!
 
void serBroadcastHandler(byte command, byte* buf) {
  
  switch(command) {
    case OM_BCAST_START:
      startProgram();
      break;
      
    case OM_BCAST_STOP:
      stopProgram();
      break;
      
    case OM_BCAST_PAUSE:
      pauseProgram();
      break;
      
    default:
      break;
  }
  
}
  
void serProgramData(byte* input_serial_buffer) {

    // used for serial response
  boolean fail = false;
  
    // look for a sub-command in the first byte of the
    // data
    
  if( input_serial_buffer[0] == 1 ) {
       fail = serProgramCamera(input_serial_buffer);
  } 
  else if( input_serial_buffer[0] == 2 ) {
      fail = serProgramMotor(input_serial_buffer); 
  }
  
  // send serial response  
 if( fail ) {
   Node.response(false);
 }
 else {
   Node.response(true);
 }
 
}
           
                 
  
void serProgramAction(byte* input_serial_buffer) {

  // used for serial response
  boolean fail = false;
  
  byte subcom = input_serial_buffer[0];
  input_serial_buffer++;
  
   switch( subcom ) {

           case 0:
                   // noop
                 break;
                 
           case 1: 
           
                  startProgram();
                  break;
           
           case 2: 

                  pauseProgram();
                  break;
                  
            case 3:
                  stopProgram();
                  break;

            case 4:
            
                   // camera on
                   
                   // we use our own variable, instead of the Camera object
                   // to block all camera actions without affecting manual camera shots
                  camera_on = true;
                  break;

            case 5:
            
                   // camera off
                  camera_on = false;
                  break;

            case 6:
      
                   // move motor now
                   
                   {
                      // how many steps to take
                    
                    byte dir = input_serial_buffer[0];
                    input_serial_buffer++;
                    
                    unsigned int steps   = Node.ntoui(input_serial_buffer);

                     // move
                    Motor.move( dir, steps ); 
                   }
                   
                   break;

            case 7:
            
                  // send a motor home
                  
                  Motor.home();
                  break;

           case 8:

                  // take a photo now
                
               Camera.expose( Node.ntoul(input_serial_buffer) );
               break;    

           case 9:
           
               // motor kill enable/disable
               
               Motor.sleep( input_serial_buffer[0] );
               break;

           case 10:
               //motor maximum step rate
               
               Motor.maxStepRate( Node.ntoui(input_serial_buffer) );
               break;

           case 11:
               //motor maximum step speed
               
               Motor.maxSpeed( Node.ntoui(input_serial_buffer) );
               break;

              // note gap from removed commands
               
           case 15:
               // set debug led state (on/off)
               
               debug_led_enable = input_serial_buffer[0];
               
                 // turn off led in case it was on an on cycle
               if( ! debug_led_enable )
                 debug_off();
               else
                 debug_on();
                 
               break;
               
           case 16:
           
             // stop motor now
              Motor.stop();
             break;
             
           case 17:
           
             // set motor microstep (1,2,4,8,16)
              Motor.ms( input_serial_buffer[0] );
             break;
           
           case 18:
           
             // set timing master
             ComMgr.master(input_serial_buffer[0]);                           
             break;
           
           case 19:
         
             // move distance with specified arrival, accel, and decel times
           
             serialComplexMove(input_serial_buffer);             
             break;
             
           case 20:
           
             // plan a move to occur across a specified number of shots in SMS mode
             serialComplexPlan(input_serial_buffer);
             break;
             
           case 21:
             // clear current plan (also clears motor history, except distance from home!)
             
             Motor.clear();
             mtpc = false;
             mt_plan = false;
             break;
             
           case 22:
             // max run time
             max_time = Node.ntoul(input_serial_buffer);
             break;
             
           case 100:

              // handling status requests

              serStatusRequest(input_serial_buffer);
                // the above function handles all serial
                // response on success and failure -- return out of
                // this function before getting to the default response
                // handling below                
              return;
              
           default: 
           
                  fail = true;
                  break;
         }

  // send serial response  
 if( fail ) {
   Node.response(false);
 }
 else {
   Node.response(true);
 }

 
 return;
}


boolean serProgramCamera(byte* input_serial_buffer) {
  
  boolean fail = false;
  byte subcom = input_serial_buffer[1];
  
  input_serial_buffer += 2;
  
     switch( subcom ) {
     
       case 1:
               // interval
             camera_delay = Node.ntoul(input_serial_buffer);
             break;
             
       case 2:
       
             { 
               
               unsigned long tm  = Node.ntoul(input_serial_buffer);
               
               Camera.exposeTime(tm);
             }
             
             break;
             
       case 3: 
       
             Camera.focusTime( Node.ntoui(input_serial_buffer) );               
             break;
             
       case 4:
       
             camera_max_shots  = Node.ntoui(input_serial_buffer);
             break;

       case 5:

             Camera.waitTime( Node.ntoui(input_serial_buffer) );
             break;
             
       case 6:
       
               // set focus w. shutter
             Camera.exposureFocus((boolean) input_serial_buffer[0]);             
             break;

       case 7:

              // camera cycle repeat count
              
             camera_repeat = input_serial_buffer[0];
 
             break;
             
           
       default:
       
             fail = true;
             break;
   } // end camera control switch
       
 return(fail);
 
}

boolean serProgramMotor(byte* input_serial_buffer) {

  boolean fail = false;
  byte subcom = input_serial_buffer[1];
  input_serial_buffer += 2;

    // determine which motor setting is requested
    
  switch( subcom ) {      

      case 1:
      
          Motor.steps( Node.ntoui(input_serial_buffer) );
          break;
          
      case 2:
      
            // was set ramp, remove later
          fail = true;
          break;
          
      case 3:
      
          Motor.dir( input_serial_buffer[0] );
          break;

      case 4:
      
            Motor.maxSteps( Node.ntoul(input_serial_buffer) );
          
          break;
          
      case 5:
                        
          Motor.enable(true);
          break;
          
      case 6:
      
          Motor.enable(false);
          break;
          
      case 7:
      
          Motor.homeSet();
          break;

      case 8:

           Motor.backlash(input_serial_buffer[0]);
           break;
           
      case 9:

            Motor.contSpeed(Node.ntof(input_serial_buffer));
          break;
      
      case 10:
      
          if( input_serial_buffer[0] == 1 )
            Motor.easing(OM_MOT_LINEAR);
          else if( input_serial_buffer[0] == 2 )
            Motor.easing(OM_MOT_QUAD);
          else if( input_serial_buffer[0] == 3 )
            Motor.easing(OM_MOT_QUADINV);
          else
            fail = true;
          
          break;
          
      case 11:
      
          Motor.continuous( input_serial_buffer[0] );             
          break;
                       
      case 12:
        
          // delay motor start
          
          motor_delay = Node.ntoul(input_serial_buffer);
          break;


      default:
        
          fail = true;
          break;
          
  } // end motor control code switch statement

 return(fail);
} 





void serStatusRequest(byte* input_serial_buffer) {
  
  switch( input_serial_buffer[0] ) {

    case 0:

      // serial api version

      Node.response( true, (byte) SERIAL_VERSION );
      
      break;
    
    case 1:
    
      // program run status
      
      Node.response( true, (byte) running );
      
      break;
      
    case 2:
    
      // current run time
      
      Node.response(true, run_time);
      
      break;
      

    case 3:
    
      // camera enabled
      
      Node.response( true, (byte) camera_on );
      
      break;

    case 4:
    
      // current shot count
      
      Node.response(true, camera_fired);
      
      break;
      
    case 5:
    
      // camera interval time
      
      Node.response(true, camera_delay);
      
      break;
      
    case 6:                  
    
      // camera exposure time
      
      Node.response(true, Camera.exposeTime());
      
      break;
      
    case 7:                  
      
      // camera post time
      
      Node.response(true, Camera.waitTime());
      
      break;
      
    case 8:
    
      // camera currently exposing
      
      Node.response( true, (byte) Camera.busy() );
      
      break;

    case 9:
    
      // motor enabled

        // get motor status for given motor
      Node.response( true, (byte) Motor.enable() );
      
      break;

    case 10:
    
      // motor dir
      
        // get motor dir for given motor
      Node.response( true, (byte) Motor.dir() );
      
      break;

    case 11:
    
      // motor steps moved
      
     Node.response( true, Motor.stepsMoved() );
      break;

    case 12:
    
      // motor distance from home
      
      Node.response( true, Motor.homeDistance() );
      
      break;


    case 13:

      Node.response(false);    
      break;                  

    case 14:                

      // motor max step
      
       Node.response( true, Motor.maxSteps() );      
      break;

    case 15:                

      // motor ramp levels - removed from protocol for now      
      Node.response(false);    
      break;

    case 16:                

      // motor backlash
      
      Node.response( true, Motor.backlash() );
      break;

    case 17:                

      // motor steps between shots
      
      Node.response( true, Motor.steps() );      
      break;
      
      // note gap from removed status commands

    case 22:
    
      // timing master      
      Node.response( true, timing_master );     
      break;
      
    default:
    
      Node.response(false);      
      break;
      
  } // end switch for current status req type


}

void serialComplexMove(byte* buf) {
   byte dir = buf[0];
   buf++;
   
   unsigned long dist  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long arrive  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long accel  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long decel  = Node.ntoul(buf);
   buf += 5; // one padding byte added

   Motor.move(dir, dist, arrive, accel, decel); 
}

void serialComplexPlan(byte* buf) {
  
   byte dir = buf[0];
     // continuous or sms? 0 = continuous, 1 = sms
   byte which = buf[1];
     // one padding byte added
   buf += 3;
   
   unsigned long steps  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long shots  = Node.ntoul(buf);
   buf += 5; // one padding byte added

   unsigned long accel  = Node.ntoul(buf);
   buf += 5; // one padding byte added
   
   unsigned long decel  = Node.ntoul(buf);
 
   if( which ) {
       // planned SMS move
     Motor.plan(shots, dir, steps, accel, decel);
     mt_plan = true;
       // always override shots here - we don't want to try to move further than planned, or waste time
       // camera_max_shots = shots;
     mtpc = false;
   }
   else {
       // planned continuous move
     mtpc = true;
     mt_plan = false;
     mtpc_dir   = dir;
     mtpc_accel = accel;
     mtpc_decel = decel;
     mtpc_arrive = shots;
     mtpc_steps  = steps;
   }
}
