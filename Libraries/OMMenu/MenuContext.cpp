/*
 * MenuContext.cpp
 *
 *  Created on: Jun 4, 2012
 *      Author: perepelitsa
 */

#include "MenuContext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>

// Workaround for http://gcc.gnu.org/bugzilla/show_bug.cgi?id=34734
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))


const unsigned int MenuContext::iParamMaxValue[NUMBER_OF_PARAMETERS] PROGMEM = {
	0,1,1,1,1, //0-4
	1,1,65535,65535,65535, //5-9
	5,0,0,0,1, //10 - 14
	2,3000,400,0x2459,50, //15-19
	2,1,1,7,1, //20-24
	100,100,100,0,0,0,0 //25-31
	};

const unsigned int MenuContext::iParamMinValue[NUMBER_OF_PARAMETERS] PROGMEM = {
		0,0,0,0,0, //0-4
		0,0,0,0,0, //5-9
		0,0,0,0,0, //10 - 14
		2,3000,400,0x2459,50, //15-19
		0,0,0,0,0, //20-24
		100,100,100,0,0,0,0 //25-31

	};

const unsigned int MenuContext::iStepParameters[NUMBER_OF_PARAMETERS] PROGMEM = {
    1,1,1,1,1,1,1,1,1,110,10,1,1,1,1,1, //0-15
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 //16-31
};

//--param20
const char MenuContext::fixedList1[3][7] PROGMEM = {
		{"None"},
		{"Camera"},
		{"Bulb"}
};//
//--param21
const char MenuContext::fixedList2[2][12] PROGMEM = {
		{"Begin"},
		{"StartOver"}
};//
//--param22
const char MenuContext::fixedList3[2][5] PROGMEM = {
		{"Yes"},
		{"No"}
};//
//param23
const char MenuContext::fixedList4[8][3] PROGMEM = {
		{"12"},//0
		{"24"},//1
		{"26"},//2
		{"28"},//3
		{"30"},//4
		{"36"},//5
		{"48"},//6
		{"60"},//7
};//
//param24
const char MenuContext::fixedList5[2][17] PROGMEM = {
		//1234567890123456
		{"Shoot-move-Shoot"},
		{"Continuous"}
};


MenuContext::MenuContext()
  : iKeyboardIncrement(1), cFocusParameter(INVALID_IDX), iStatus(0)
{

	for (uint8_t i=0; i < NUMBER_OF_TIMERS; i++) {
		periodTimers[i]=0;
	}

	for (uint8_t i=0; i < NUMBER_OF_PARAMETERS; i++) {
		iParamValue[i] = i;
		}
	for (uint8_t i=0; i < 6; i++) {
			sprintf(&dynList[i][0], "entry_%d", i);
		}

}


/**
 *   1. Lead-in time (in seconds]
 *   2. Lead-out time (in seconds]
 *   3. Acceleration time (in seconds]
 *   4. Deceleration time (in seconds]
 *   5. Exposure time (in milliseconds]
 *   6. "film length" of film
 *
 *   Lists:
 *   20
 * */
uint8_t MenuContext::formatParameterText(uint8_t idx, uint8_t* cLineBuf)
{
   uint8_t len = 0;
   //temporary init as
   cLineBuf[0] = '[';
   cLineBuf[1] = '=';
   char* ptrStart = (char*)&cLineBuf[2];
	if ((idx > 0) && (idx < 10))  {
	//integer type
	   sprintf(ptrStart, "%d", iParamValue[idx] );
   }
   if (idx == 10) {
	   uint16_t listIdx = iParamValue[idx];
	   checkParamRange(idx, &listIdx);
	   sprintf(ptrStart, "%s", &dynList[listIdx][0] );
   }
  /* magic 11(12)(13) or 14:15:16 is time type*/
   if ((idx == 11) || (idx == 14)){
	   sprintf(ptrStart, "%02d:%02d:%02d", iParamValue[idx], iParamValue[idx+1], iParamValue[idx+2]);
   }
   /*magic 20,21... is list type*/
   if ((idx >= 20) && (idx < NUMBER_OF_PARAMETERS))
   {
	   uint16_t listIdx = iParamValue[idx];
	   //checkParamRange(idx, &listIdx);
	   listIdx = 0;
       if (idx == 20){
         strcpy_P(ptrStart, &fixedList1[listIdx][0]);
       } else if (idx == 21){
    	 strcpy_P(ptrStart, &fixedList2[listIdx][0]);
       } else if (idx == 22){
      	 strcpy_P(ptrStart, &fixedList3[listIdx][0]);
       } else if (idx == 23){
   	     strcpy_P(ptrStart, &fixedList4[listIdx][0]);
       } else if (idx == 24){
      	 strcpy_P(ptrStart, &fixedList5[listIdx][0]);
       } else {
    	  // strcpy_P((char*)cLineBuf, PSTR("--noList--"));
       }
   }
   len = strlen((char*)cLineBuf);
   return len;
}

/**
 *
 * */
void  MenuContext::setKeyboardCode(uint8_t key)
{
	keyCode = key;
}

/**
 *
 * */
void MenuContext::openParameter(uint8_t idx){
	cFocusParameter = idx;
	iModifiableValue = iParamValue[idx];
}

/***
 *
 * */
bool MenuContext::isParamEdit()
{
	return (cFocusParameter != INVALID_IDX);
}

/**
 *
 * */
void MenuContext::incParameter(){

	if((iModifiableValue+(iStepParameters[cFocusParameter]*iKeyboardIncrement))
			>= iParamMaxValue[cFocusParameter])
	  {
		iModifiableValue = iParamMaxValue[cFocusParameter];
	  }
	else {
		iModifiableValue += (iStepParameters[cFocusParameter]*iKeyboardIncrement);
	  }
}
/*
 *
 * */
void MenuContext::decParameter(){
	if((iModifiableValue-(iStepParameters[cFocusParameter]*iKeyboardIncrement))
			<= iParamMinValue[cFocusParameter])
		{
			iModifiableValue = iParamMinValue[cFocusParameter];
		}
		else {
			iModifiableValue -= (iStepParameters[cFocusParameter]*iKeyboardIncrement);
		}

}
/*
 *
 * */
unsigned int MenuContext::closeParameter(bool saveFlag){
	if (saveFlag) {
		iParamValue[cFocusParameter] = iModifiableValue;
	}
	cFocusParameter = INVALID_IDX;
	return iModifiableValue;
}

/**
 *
 * */
void MenuContext::setTimer(uint8_t idx, unsigned int value){
	periodTimers[idx] = value;
}

/**
 *
 * */
void MenuContext::decTimer(uint8_t idx){
	periodTimers[idx]--;
}


uint8_t MenuContext::checkParamRange(uint8_t idx, uint16_t* pParam)
{
	uint8_t result = 0;
	uint16_t paramMinValue;
	uint16_t paramMaxValue;
	//copy across FLASH space
	memcpy_P(&paramMinValue, &iParamMinValue[idx], sizeof(paramMinValue));
	memcpy_P(&paramMaxValue, &iParamMinValue[idx], sizeof(paramMaxValue));

	if (*pParam < paramMinValue) {
		*pParam = paramMinValue;
	} else if (*pParam > paramMaxValue) {
		*pParam = paramMaxValue;
	} else {
		result = 1;
	}
	return result;
}

