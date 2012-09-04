/*
 * UserInput.h
 *
 *  Created on: Jun 4, 2012
 *      Author: perepelitsa
 */

#ifndef USERINPUT_H_
#define USERINPUT_H_

#include <inttypes.h>
#include "Arduino.h"
#include "MenuContext.h"

//KEYBOARD CONSTANTS
#define TYPEMATIC_INITIAL 10	//1 sec before first repeat
#define TYPEMATIC 2				//200mS for every increment thereafter
#define TYPEMATIC_STEP 10		//10 increments before going up a multiplier

//bit masks for keys
#define K_ENTER 1				//Keyboard Code associated with a key
#define K_MENU 2				//Keyboard Code associated with a key
#define K_UP 4					//Keyboard Code associated with a key
#define K_DOWN 8				//Keyboard Code associated with a key

#define KEYBOARD_TIMEOUT 1200	//2 minutes for keyboad before timeout
/**
 * Scans user input, like keyboard, joystick, etc.
 * Debounce reading and sets flag on ready.
 * */
class UserInput {
	uint8_t _up_pin; //
	uint8_t _down_pin; //
	uint8_t _enter_pin; //
	uint8_t _menu_pin; //

	uint8_t cScanCode;
	uint8_t iKeyboardIncrement;
	uint8_t cKeyboardLoopCounter;
	uint8_t cPB[3];	//for debounce of pushbuttons

	uint8_t cKeyboardCode;

protected:
	uint8_t sampleBtnPins(void);

public:
	UserInput(uint8_t up, uint8_t down, uint8_t enter, uint8_t menu);
	uint8_t KeyboardRead(MenuContext& status);
};

#endif /* USERINPUT_H_ */
