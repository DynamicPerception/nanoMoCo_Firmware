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
   
   **** See the included PDF file with this source distribution for protocol commands. ****

  For all requests that set a value or trigger a command,
  (i.e. not status requests) the response data len will be
  0.  
  
  For all status requests, the response data len will be
  greater than or equal to 1.  
       
*/
    
    
/* Handles Node 1 Commands

  only Node1 goes through this function, determines which node
  to respond to

  */

void serNode1Handler(byte subaddr, byte command, byte*buf) {
  node = 1;
  serCommandHandler(subaddr, command, buf);
}

/* Handles Node 2 Commands

  only NodeBlue goes through this function, determines which node
  to respond to

  */

void serNodeBlueHandler(byte subaddr, byte command, byte*buf) {
  node = 2;
  serCommandHandler(subaddr, command, buf);
}


/* Handles Node Packets not for this device

  only Node goes through this function, repeats the packet to NodeBlue

  */

void serNotUsNode1Handler(byte addr, byte subaddr, byte command, byte bufLen, byte*buf) {
  /*
  USBSerial.print("Packet sent out is ");
  USBSerial.print(addr);
  USBSerial.print("  ");
  USBSerial.print(subaddr);
  USBSerial.print("  ");
  USBSerial.print(command);
  USBSerial.print("  ");
  USBSerial.print(bufLen);
  USBSerial.print("  ");
  
  for (int i = 0; i < bufLen; i++)
    USBSerial.println(buf[i]);
    */
  /*char message[512];
  sprintf(message,"serNotUsNode1Handler: %p %d %d %d %d\n", &NodeBlue, addr, subaddr, command, bufLen);
  
  USBSerial.print(message);
  */
  //delay(10);
  NodeBlue.sendPacket(addr,subaddr,command,bufLen,buf);
}


/* Handles Node 2 Commands

  only NodeBlue goes through this function, repeats the packet to Node1

  */

void serNotUsNodeBlueHandler(byte addr, byte subaddr, byte command, byte bufLen, byte*buf) {
  /*char message[512];
  sprintf(message,"serNotUsNode1Handler: %p %d %d %d %d\n", &Node, addr, subaddr, command, bufLen);
  USBSerial.print(message);*/
  /*USBSerial.print("Packet sent out is ");
  USBSerial.print(addr);
  USBSerial.print("  ");
  USBSerial.print(subaddr);
  USBSerial.print("  ");
  USBSerial.print(command);
  USBSerial.print("  ");
  USBSerial.print(bufLen);
  USBSerial.print("  ");
  for (int i = 0; i < bufLen; i++)
    USBSerial.println(buf[i]);
  */
//  delay(10);
  Node.sendPacket(addr,subaddr,command,bufLen,buf);
}
    

/* Handle normal commands

  2 = program control commands
  3 = data setting commands
 
  all else generates error
  */
  
void serCommandHandler(byte subaddr, byte command, byte* buf) {

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
       if (node == 2)
         NodeBlue.response(false);
       else
         Node.response(false);
       break;
 }

}

 // handle broadcast commands - do not send any
 // responses to a broadcast command!
 
void serBroadcastHandler(byte subaddr, byte command, byte* buf) {
  
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
   if (node == 2)
     NodeBlue.response(false);
   else
     Node.response(false);
 }
 else {
   if (node == 2)
     NodeBlue.response(true);
   else
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
                    
                    unsigned int steps   = Node.ntoul(input_serial_buffer);

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
                 debugOff();
               else
                 debugOn();
                 
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
             
           case 23:
             // store device name
             
             // I am not the trusting type, so we'll scan the buffer rather than
             // assume that it is a null-terminated string...
             
             {
               memset(device_name, 0, 16);
               
               for(byte i = 0; i <= 15; i++) {
                if( input_serial_buffer[i] != 0 ) 
                   device_name[i] = input_serial_buffer[i];
               }
             }
             
               // save to eeprom
               
             OMEEPROM::write(EE_NAME, *device_name, 16);
               
             break;
             
           case 24: 
             // set common line for step pulsing
             
             if( input_serial_buffer[0] > 2 ) {
               fail = true;
             }
             else {
               if( input_serial_buffer[0] == 0 )
                 ComMgr.stopWatch();
               else
                 ComMgr.watch(input_serial_buffer[0]);
             }
             
             break;
             
           case 25: 
             // set autpause (only if using planned moves)
             
             if( ! mt_plan )
               fail = true;
             else
               control_autoPause = input_serial_buffer[0];
               
             break;
                  
           case 26:
             // step forward one interleaved (sms) plan cycle
       
             if( ! mt_plan ) {
               fail = true;
             }
             else {
                 // dig this?  We advance one frame by simply turning on autopausing
                 // and then playing.  This is a convienence function for clients, 
                 // rather than forcing them to run both commands.
               
                 // go ahead and make sure we fire immediately
               camera_tm = millis() - camera_delay;
               
               control_autoPause = true;
               startProgram();
             }
     
             break;
             
           case 27:
             // step back one interleaved (sms) plan cycle
           
               // return an error if we don't actually have a planned move
             if( ! mt_plan )
               fail = true;
             else {
               if( motor_delay > 0 && camera_fired < motor_delay ) {
                   // do not reverse the plan, the motor isn't supposed to move here
               }
               else {
                   // rollback one shot in the program
                   if( camera_fired > 0 )
                     camera_fired--;
                     
                   Motor.planReverse();
               }
               
                 // need to decrease run time counter
               {
                 unsigned long delayTime = ( camera_delay > (Camera.exposeTime() + Camera.focusTime() + Camera.waitTime()) ) ? camera_delay : (Camera.exposeTime() + Camera.focusTime() + Camera.waitTime());
                 
                 if( run_time >= delayTime )
                   run_time -= delayTime;
                 else
                   run_time = 0;
               }
               
             } // end else (mt_plan
               
             break;
             
           case 28:
           //Flashes debug pin to identify controller 
             
             flasher(AEN_PIN, 4);
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
   if (node == 2)
      NodeBlue.response(false);
   else
     Node.response(false);
 }
 else {
   if (node == 2)
     NodeBlue.response(true);
   else
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
       
             Camera.exposeTime( Node.ntoul(input_serial_buffer) );
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
      if (node == 2)
        NodeBlue.response( true, (byte) SERIAL_VERSION );
      else
        Node.response( true, (byte) SERIAL_VERSION );
      
      break;
    
    case 1:
    
      // program run status
      if (node == 2)
        NodeBlue.response( true, (byte) running );
      else      
        Node.response( true, (byte) running );
      
      break;
      
    case 2:
    
      // current run time
      if (node == 2)
        NodeBlue.response(true, run_time);
      else  
        Node.response(true, run_time);
      
      break;
      

    case 3:
    
      // camera enabled
      
      if (node == 2)
        NodeBlue.response( true, (byte) camera_on );
      else  
        Node.response( true, (byte) camera_on );
      
      break;

    case 4:
    
      // current shot count
      if (node == 2)
        NodeBlue.response(true, camera_fired);
      else  
        Node.response(true, camera_fired);
      
      break;
      
    case 5:
    
      // camera interval time
      
      if (node == 2)
        NodeBlue.response(true, camera_delay);
      else 
        Node.response(true, camera_delay);
      
      break;
      
    case 6:                  
    
      // camera exposure time
      if (node == 2)
        NodeBlue.response(true, Camera.exposeTime());
      else 
        Node.response(true, Camera.exposeTime());
      
      break;
      
    case 7:                  
      
      // camera post time
      
      Node.response(true, Camera.waitTime());
      
      break;
      
    case 8:
    
      // camera currently exposing
      
      if (node == 2)
        NodeBlue.response( true, (byte) Camera.busy() );
      else
        Node.response( true, (byte) Camera.busy() );
      
      break;

    case 9:
    
      // motor enabled

        // get motor status for given motor
      if (node == 2)
        NodeBlue.response( true, (byte) Motor.enable() );
      else  
        Node.response( true, (byte) Motor.enable() );
      
      break;

    case 10:
    
      // motor dir
      
        // get motor dir for given motor
        
      if (node == 2)
        NodeBlue.response( true, (byte) Motor.dir() );
      else 
        Node.response( true, (byte) Motor.dir() );
      
      break;

    case 11:
    
      // motor steps moved
     if (node == 2)
       NodeBlue.response( true, Motor.stepsMoved() );
     else 
       Node.response( true, Motor.stepsMoved() );
      break;

    case 12:
    
      // motor distance from home
      
      if (node == 2)
        NodeBlue.response( true, Motor.homeDistance() );
      else 
        Node.response( true, Motor.homeDistance() );
      
      break;


    case 13:
      
      if (node == 2)
        NodeBlue.response(false);
      else 
        Node.response(false);    
      break;                  

    case 14:                

      // motor max step
      
      if (node == 2)
        NodeBlue.response( true, Motor.maxSteps() ); 
      else 
        Node.response( true, Motor.maxSteps() );      
      break;

    case 15:                

      // motor ramp levels - removed from protocol for now  
      if (node == 2)
        NodeBlue.response(false);  
      else     
        Node.response(false);    
      break;

    case 16:                

      // motor backlash
      if (node == 2)
        NodeBlue.response( true, Motor.backlash() );
      else  
        Node.response( true, Motor.backlash() );
      break;

    case 17:                

      // motor steps between shots
      
      if (node == 2)
        NodeBlue.response( true, Motor.steps() );  
      else 
        Node.response( true, Motor.steps() );      
      break;
      
      // note gap from removed status commands

    case 22:
    
      // timing master    
      if (node == 2)
        NodeBlue.response( true, timing_master );   
      else   
        Node.response( true, timing_master );     
      break;
      
    case 23:
    
      // device name
      if (node == 2)
        NodeBlue.response(true, (char*)device_name, 16);   
      else       
        Node.response(true, (char*)device_name, 16);
      break;
      
    default:
      
      if (node == 2)
        NodeBlue.response(false);    
      else  
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

/*
/*=========================================
          Node Response Functions
===========================================

void response(bool p_stat){
  if (node == 2)
    NodeBlue.response(p_stat);    
  else  
    Node.response(p_stat); 
} 

void response(bool p_stat, uint8_t p_resp){
    if (node == 2)
    NodeBlue.response(p_stat, p_resp);    
  else  
    Node.response(p_stat, p_resp); 
}

void response(bool p_stat, unsigned int p_resp){
    if (node == 2)
    NodeBlue.response(p_stat, p_resp);    
  else  
    Node.response(p_stat, p_resp); 
}

void response(bool p_stat, int p_resp){
    if (node == 2)
    NodeBlue.response(p_stat, p_resp);    
  else  
    Node.response(p_stat, p_resp); 
}

void response(bool p_stat, unsigned long p_resp){
    if (node == 2)
    NodeBlue.response(p_stat, p_resp);    
  else  
    Node.response(p_stat, p_resp); 
}

void response(bool p_stat, long p_resp){
    if (node == 2)
    NodeBlue.response(p_stat, p_resp);    
  else  
    Node.response(p_stat, p_resp); 
}

void response(bool p_stat, float p_resp){
    if (node == 2)
    NodeBlue.response(p_stat, p_resp);    
  else  
    Node.response(p_stat, p_resp); 
}

void response(bool p_stat, char* p_resp, int p_len){
    if (node == 2)
    NodeBlue.response(p_stat, p_resp, p_le);    
  else  
    Node.response(p_stat, p_resp, p_le); 
}

*/



