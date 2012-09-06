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
  {}, //2
  {}, //3
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
{"Frames/sec slct "},//4
{"  Axis to move? "},//5
{"Move Start point"},//6
{"Move End point  "},//7
{"Lead-in time    "},//8
{"Lead-out time   "},//9
{"Accel. time     "},//10
{"Decel. time     "},//11
{"Enter RT of film"},//12
{"Exposure control"},//13
{"Exposure time   "},//14
{"Motion mode     "},//15
{"Film length     "},//16
{"Begin or Again? "},//17
{"Create Film wzrd"},//18
{"  Settings      "},//19
{"  Re-scan bus   "},//20
{"Camera control? "},//21
{"----------      "},//22
{"String 24       "},//23
{"     MENU       "},//24
{"                "}//25!!blanc
};

/**
 *
 * */
const Screen Resources::screens[] PROGMEM = {
//INITIAL_DISPLAY_SCREEN
		{ 2, //0-INITIAL_DISPLAY=4 lines
		  0, //
		  { //array
			{ 0, 0, MENU_LEVEL_ENTRY, MENU_SCREEN, 0, 0 }, //1
			{ 1, 0, MENU_LEVEL_ENTRY, MENU_SCREEN, 0, 0 }, //2
		  }
		},
		//MENU_SCREEN-> Screen[1]
		{
		 4,							//
		 1,							//arrows required
		 {
		    {24,0,STATIC_ITEM_ENTRY, 0,0,0},	//1
		    {17,0,MENU_LEVEL_ENTRY, CREATE_FILM_WIZARD,0,0},	//2
		    {18,0,MENU_LEVEL_ENTRY, RESCAN_DEVICES},	//3
		    {19,0,MENU_LEVEL_ENTRY, SETTINGS_SCREEN},	//4
		  }
		},
		//CREATE_FILM_WIZARD-> Screen[2]
		{
		 4,							//=4 lines
		 1,
		  {
			 {12,0,STATIC_ITEM_ENTRY,0,0,0},	//1
			 {13,0,WIZARD_LEVEL_ENTRY,RESCAN_DEVICES,3,0},	//2
			 {14,0,WIZARD_LEVEL_ENTRY,0,4,0},	//3
			 {15,0,WIZARD_LEVEL_ENTRY,0,0,0}	//4
		  }
		},
		//RESCAN_DEVICES
		{
		  2,							//=4 lines
		  0,                         //arrows not required
		  {
			{16,0,STATIC_ITEM_ENTRY,0,0,0},	//1
			{BLANK_MESSAGE, OP_EXPAND_LIST, PARAM_ITEM_ENTRY, 0, 0},	//2
		  }
		},
		//SETTINGS_SCREEN
		{
				 4,							//=4 lines
				 1,
				{
				   {12,0,PARAM_ITEM_ENTRY,0,2,0},	//1
				   {13,0,PARAM_ITEM_ENTRY,0,3,0},	//2
				   {14,0,PARAM_ITEM_ENTRY,0,4,0},	//3
				   {15,0,PARAM_ITEM_ENTRY,0,0,0}	//4

				 }
		},

		//SUB_WIZARD
		{
		  4,							//=4 lines
		  1,
			{
			   {12,0,PARAM_ITEM_ENTRY,0,2,0},	//1
			   {13,0,PARAM_ITEM_ENTRY,0,3,0},	//2
			   {14,0,PARAM_ITEM_ENTRY,0,4,0},	//3
			   {15,0,PARAM_ITEM_ENTRY,0,0,0}	//4
			 }
		}
};




