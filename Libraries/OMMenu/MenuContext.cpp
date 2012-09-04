/*
 * MenuContext.cpp
 *
 *  Created on: Jun 4, 2012
 *      Author: perepelitsa
 */

#include "MenuContext.h"


MenuContext::MenuContext() {
	iStatus = 0;
	for (uint8_t i=0; i < NUMBER_OF_TIMERS; i++) {
		periodTimers[i]=0;
	}
	iKeyboardIncrement = 1;
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
	iModifiableValue = iParameter[idx];
}

/**
 *
 * */
void MenuContext::incParameter(){

	if((iModifiableValue+(iStepParameters[cFocusParameter]*iKeyboardIncrement))
			>= iMaximumParameters[cFocusParameter])
	  {
		iModifiableValue = iMaximumParameters[cFocusParameter];
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
			<= iMinimumParameters[cFocusParameter])
		{
			iModifiableValue = iMinimumParameters[cFocusParameter];
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
		iParameter[cFocusParameter] = iModifiableValue;
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


