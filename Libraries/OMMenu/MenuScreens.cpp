/*
 * MenuScreens.cpp
 *
 *  Created on: 13.07.2012
 *      Author: perepelitsa
 */
#include "MenuScreens.h"

// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))

const char Resources::d[] PROGMEM = "Flash string1";


const char Resources::glyphs[8][8] PROGMEM = {
  {0x1f, 0x1b, 0x11, 0x0a, 0x1b, 0x1b, 0x1b, 0x1f }, //0
  {0x1f, 0x1b, 0x1b, 0x1b, 0x0a, 0x11, 0x1b, 0x1f }, //1
  {0x10, 0x08, 0x0C, 0x1F, 0x1F, 0x0C, 0x08, 0x10}, //2
  {0x01, 0x02, 0x06, 0x1F, 0x1F, 0x06, 0x02, 0x01}, //3
  {}, //4
  {}, //5
  {}, //6
  {} //7
};


const char Resources::resources [NUMBER_OF_STRINGS][LCD_WIDTH+1] PROGMEM = {
//1234567890123456
{"   Hello Demo!! "},//0
{" <OK> for Menu  "},//1
{"   Stopped..    "},//2
{" Running [%d/%d]"},//3
{"  Frames/sec    "},//4
{"  Axis to move? "},//5
{" Move to Start  "},//6
{" Move to End    "},//7
{" Lead-in time   "},//8
{" Lead-out time  "},//9
{" Accel. time    "},//10
{" Decel. time    "},//11
{" Set film RTime "},//12
{" Exposure ctrl  "},//13
{" Exposure time  "},//14
{" Motion mode    "},//15
{" Film length    "},//16
{" Begin / Again? "},//17
{"  Create Film   "},//18
{"  Settings      "},//19
{"  Re-scan bus   "},//20
{" Camera control "},//21
{" String 2       "},//22
{" String 1       "},//23
{"     MENU       "},//24
{"                "}//25!!blanc
};


/**
 *
 * */
const MenuScreen Resources::screens[] PROGMEM = {
//INITIAL_DISPLAY_SCREEN
		{ 2, //0-INITIAL_DISPLAY=4 lines
		  0, //
		  DIALOG_LEVEL_ENTRY,
		  0,
		  { //array
			{ 0, 0, MENU_SCREEN, 0 }, //1
			{ 1, 0, MENU_SCREEN, 0 }, //2
		  }
		},
		//MENU_SCREEN-> Screen[1]
		{
		 4,							//
		 1,							//arrows required
		 MENU_LEVEL_ENTRY,
		 0,
		 {
		    {24, 0, 0, 0},	//1
		    {18, 0, CREATE_FILM_WIZARD, 0},	//2
		    {20, 0, RESCAN_DEVICES, 0},	//3
		    {19, 0, SETTINGS_SCREEN, 0},	//4
		  }
		},
		//CREATE_FILM_WIZARD-> Screen[2]
		{
		 9,							//=4 lines
		 1,
		 WIZARD_LEVEL_ENTRY,
		 0,
		  {//items
			 {0,0,0,0}, //dummy header
			 {5, 0,OPEN_EDIT, 10},  //1.Select Axis to move
			 {12,0,OPEN_EDIT,11}, //2. Enter "real time" of film
			 {13,0,OPEN_EDIT,20}, //3. Select exposure control
			 {14,0,OPEN_EDIT,2}, //4. Input exposure time in mS
			 {15,0,OPEN_EDIT,24}, //5. Select Motion Mode
			 {16,0,OPEN_EDIT,3}, //6. Enter "film length" of film
			 {17,0,OPEN_EDIT,21}  //7. Begin Film or Start Over?
		  }
		},
		//RESCAN_DEVICES
		{
		  2,							//=4 lines
		  0,                         //arrows not required
		  MENU_LEVEL_ENTRY,
		  0,
		  {
		    {0,0,0,0}, //dummy header
			{16,0,0,0}	//1
		  }
		},
		//SETTINGS_SCREEN
		{
			3,							//=4 lines
			1,
			WIZARD_LEVEL_ENTRY,
			0,
			  {
				 {0,0,0,0},
				 {4,0,OPEN_EDIT, 3},	//2
				 {21,0,OPEN_EDIT,4}	//3
			  }
		},
		//AXIS_WIZARD
		{
			8,							//=items
			1,
			WIZARD_LEVEL_ENTRY,
			0,
			  {
				 {0,0,0,0}, //dummy header
				 {6,1,OPEN_ACTION,4},   //1.1 Move axis to start point
				 {7,1,OPEN_ACTION,5},	//1.2 Move axis to end point
				 {8,0,OPEN_EDIT,6},	    //1.3 Lead-in time
				 {9,0,OPEN_EDIT,7},     //1.4 Lead-out time
				 {10,0,OPEN_EDIT,8},    //1.5 Acceleration time
				 {11,0,OPEN_EDIT,9},    //1.6 Deceleration time
				 {17,0,OPEN_EDIT,22}    //More?
			  }
		},




};




