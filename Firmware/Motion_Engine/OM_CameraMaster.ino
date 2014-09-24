/*

 Camera Functionality Library

 OpenMoco MoCoBus Core Libraries

 See www.dynamicperception.com for more information

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
void stopAllCameras() {

	// stop timer (always)
	// we do this before bringing the pin low in case
	// a VERY short action time was set to prevent the
	// timer from continuing to trigger and getting stuck in
	// a loop.

	MsTimer2::stop();
	camera[0].stop();
	camera[1].stop();

}


/** Trigger Exposure

 This method triggers an exposure for the amount of time as set via exposeTime().
 Sends an expose begin signal.

*/

/*

void exposeAll() {
	camera[0].expose();
	camera[1].expose();
}

/** Trigger Exposure

 This method triggers an exposure for the amount of time specified.
 Sends an expose begin signal.

 @param p_Time
 Length (in milliseconds) of the exposure.

*/
/*
void expose(unsigned long p_time) {

	// do not expose if exposure time is zero
  if( p_time == 0 ) {
  	  if( f_camSignal != 0 )
  	  	  f_camSignal(OM_CAM_EFIN);

  	  return;
  }
  	// indicate (for stop()), that we are in an exposure
  	// to properly set output states when timer2 completes

    m_curAct = OM_CAM_INEXP;


    // determine if focus pin should be brought high
    // w. the shutter pin (for some nikons, etc.)

  if( m_focusShut == true )
    digitalWrite(m_focus, HIGH);

  digitalWrite(m_shutter, HIGH);

    // start timer to stop camera exposure
  MsTimer2::set(p_time, OMCameraFunction::stop);
  MsTimer2::start();


    // update camera currently engaged
  m_isBzy = true;


    // if there is a pointer to a function
    // to be called when the camera is done,
    // do it now

  if( f_camSignal != 0 )
  	  f_camSignal(OM_CAMEXP);

  return;
}


/** Trigger Focus

 This method triggers a focus for the amount of time as set via focusTime().
 Sends a focus begin signal.

*/
/*
void focus() {
	this->focus(m_timeFoc);
}

/** Trigger Focus

 This method triggers a focus for the amount of time specified.
 Sends a focus begin signal.

 @param p_Time
 Length (in milliseconds) of the focus.

*/
/*
void focus(unsigned int p_time) {

	// do not focus if focus time is 0
  if( p_time == 0 ) {
  	  if( f_camSignal != 0 )
  	  	  f_camSignal(OM_CAM_FFIN);

  	  return;
  }

  digitalWrite(m_focus, HIGH);

    // start timer to stop focus engage

  m_curAct = OM_CAM_INFOC;

  MsTimer2::set(p_time, OMCameraFunction::stop);
  MsTimer2::start();
    // update camera currently engaged
  m_isBzy = true;
    // report focus in progress
  if( f_camSignal != 0 )
  	  f_camSignal(OM_CAMFOC);

}


/** Trigger Delay

 This method triggers a delay action for the amount of time specified.
 Sends a delay begin signal.

 @param p_Time
 Length (in milliseconds) of the delay.

*/
/*
void cameraWait(unsigned int p_Time) {

	// do not wait, if wait time is 0
  if( p_Time == 0 ) {
	  if( f_camSignal != 0 )
		  f_camSignal(OM_CAM_WFIN);

	  return;
  }

  m_curAct = OM_CAM_INDLY;

  MsTimer2::set(p_Time, OMCameraFunction::stop);
  MsTimer2::start();

    // update camera currently engaged
  m_isBzy = true;

  if( f_camSignal != 0 )
  	  f_camSignal(OM_CAMWAIT);
}

