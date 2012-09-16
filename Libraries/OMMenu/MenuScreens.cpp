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
{" OMMenu idle    "},//1
{"      is on     "},//2
{" Time: @t       "},//3
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
{" Begin or Again "},//17
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
		  MENU_LEVEL_ENTRY,
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
		    {24,0,0,0},	//1
		    {18,0,CREATE_FILM_WIZARD,0},	//2
		    {20,0,RESCAN_DEVICES,0},	//3
		    {19,0,SETTINGS_SCREEN,0},	//4
		  }
		},
		//CREATE_FILM_WIZARD-> Screen[2]
		{
		 2,							//=4 lines
		 1,
		 WIZARD_LEVEL_ENTRY,
		 0,
		  {//items
			 {0,0,0,0}, //dummy header
			 {5, LIST_SELECTOR, AXIS_WIZARD, 10}  //
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
			PARAM_ITEM_ENTRY,
			0,
			{
			   {0,0,0,0},               //dummy header
			   {4,0,LIST_SELECTOR, 3},	//1
			   {21,0,LIST_SELECTOR,4}	//2
		    }
		},
		//LIST_SELECTOR
		{
			3,							//=4 lines
			1,
			PARAM_ITEM_EDIT,
			0,
			  {
				 {0,0,0,0},
				 {4,0,LIST_SELECTOR, 3},	//2
				 {21,0,LIST_SELECTOR,4}	//3
			  }
		},
		//AXIS_WIZARD
		{
			7,							//=items
			1,
			WIZARD_LEVEL_ENTRY,
			0,
			  {
				 {0,0,0,0}, //dummy header
				 {6,0,0,1},
				 {7,0,0,2},	//2
				 {8,0,0,3},	//3
				 {9,0,0,4},
				 {10,0,0,5},
				 {11,0,0,6}
			  }
		},

		//MAIN_WIZARD_2
		{
					7,							//=items
					1,
					WIZARD_LEVEL_ENTRY,
					0,
					{
						 {0,0,0,0}, //dummy header
						 {12,0,0,1},
						 {13,0,0,2},
						 {14,0,0,3},
						 {15,0,0,4},
						 {16,0,0,5},
						 {17,0,0,6}
					}
		},


};




