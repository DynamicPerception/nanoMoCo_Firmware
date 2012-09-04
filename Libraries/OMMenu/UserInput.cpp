/*
 * UserInput.cpp
 *
 *  Created on: Jun 4, 2012
 *      Author: perepelitsa
 */

#include "UserInput.h"

/**
 * Non-virtual destructor, change if wants to grow
 * */
UserInput::UserInput(uint8_t up, uint8_t down, uint8_t enter, uint8_t menu)
{
	cPB[0]=cPB[1]=cPB[2]=0;
	cScanCode = 0x55;
	_up_pin = up; //
	_down_pin = down; //
	_enter_pin = enter; //
	_menu_pin = menu; //
	//
	pinMode(_up_pin, INPUT);
	pinMode(_down_pin, INPUT);
	pinMode(_enter_pin, INPUT);
	pinMode(_menu_pin, INPUT);

	pinMode(13, OUTPUT);

	// enable internal pull-up resistors
	digitalWrite(_up_pin, HIGH);
	digitalWrite(_down_pin, HIGH);
	digitalWrite(_enter_pin, HIGH);
	digitalWrite(_menu_pin, HIGH);

	digitalWrite(13,HIGH);
}

/**
 * For better portability. Use digitalRead instead of
 * N.B. Read is skew i.e. not all pins at same time
 * return zero bits for pressed buttons
 * */
uint8_t UserInput::sampleBtnPins()
{
  uint8_t scan = 0;

  if (digitalRead(_up_pin)) {
	  bitSet(scan, 3); //match K_UP;
  }

  if (digitalRead(_down_pin)) {
	  bitSet(scan, 2);// |= K_DOWN;
  }

  if (digitalRead(_enter_pin)) {
	 bitSet(scan, 0); // match K_ENTER;
  }

  if (digitalRead(_menu_pin)) {
	  bitSet(scan, 1); //match K_MENU;
  }
  return scan;
}

/***
 *
 *
 * */
uint8_t UserInput::KeyboardRead(MenuContext& status)
{
	//shunt last readings
	cPB[0]=cPB[1];
	cPB[1]=cPB[2];
	cPB[2]= ~sampleBtnPins();
	//3 scans for debounce
	if ((cPB[0]==cPB[1]) && (cPB[0]==cPB[2]))
	{
		if (cScanCode != cPB[2])
		{
			status.setContextBits(PB_DELTA);
			cScanCode = cPB[2];
		}
	}

	if (status.getContext() & PB_DELTA)
	{//here there is a keyboard input
		status.clrContextBits(PB_DELTA);
		//clear the calling flag
		//and start the timer with the initial period, i.e. for the 1st 1 sec nothing happens
		status.setTimer(T_KEYBOARD_REPEAT, TYPEMATIC_INITIAL);
		iKeyboardIncrement = 1;		//inititlise the increment
		//(it can be 1, 10, 100, & 1000 & 10000)
		cKeyboardLoopCounter = 1;
		//number of loops before the KeyboardIncrement is incremented
		status.setContextBits(KEYBOARD_VALID);
		status.setTimer(T_KEYBOARD_TIMEOUT, KEYBOARD_TIMEOUT);
		//refresh timer

		//table defined scanCode to KeyCode translation
		if (bitRead(cScanCode,0)) {
			cKeyboardCode = K_ENTER;
		} else if (bitRead(cScanCode,1)) {
			cKeyboardCode = K_MENU;
		} else if (bitRead(cScanCode,2)) {
			cKeyboardCode = K_UP;
		} else if (bitRead(cScanCode,3)) {
			cKeyboardCode = K_DOWN;
		} else {
			cKeyboardCode = 0x00;
			status.clrContextBits(KEYBOARD_VALID);
				//not a valid code
		}

	} else {
	//no keyboard input, check for typematic increases
		if (((cKeyboardCode==K_UP) || (cKeyboardCode==K_DOWN)) && (cScanCode !=0xf8))
		{//only if up or down
			if (status.getTimer(T_KEYBOARD_REPEAT) == 0)
			{// only when time is up
				cKeyboardLoopCounter++;
				if (cKeyboardLoopCounter > TYPEMATIC_STEP)
				{
					cKeyboardLoopCounter=1;
					if (iKeyboardIncrement < 10000)
					{
						iKeyboardIncrement *=10;
					}
				}
				status.setTimer(T_KEYBOARD_REPEAT, TYPEMATIC);
				status.setContextBits(KEYBOARD_VALID);
			}
		}
	}
	status.setKeyboardCode(cKeyboardCode);
	return cKeyboardCode;
}
