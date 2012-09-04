/*
 * HorizMenu.h
 *
 *  Created on: 04.09.2012
 *      Author: perepelitsa
 */

#ifndef HORIZMENU_H_
#define HORIZMENU_H_

#include <inttypes.h>

#include "MenuContext.h"
#include "MenuScreens.h"
#include "LiquidCrystal.h"

#define LEFT_ARROW_GLYPH 3
#define RIGHT_ARROW_GLYPH 4

class HorizMenu {

	struct MenuBufferType {
		//This is the memory for the displays sized
		//for the maximum number of rows
		uint8_t cArrowsRequired;	//need to display selection arrows
		uint8_t cCurrentScreen; //in order to recreate later
		uint8_t cParamNum[MAX_SCREEN_SEL];	//parameter associated with line
		uint8_t cNextAction[MAX_SCREEN_SEL];	//where to go when enter is pressed
		uint8_t cAssociatedDisplay[MAX_SCREEN_SEL];	//display to invoke when enter is pressed
		//this can carry a number associated with the line, so that the lines can be packed
		//and still correctly identified (it could be eeprom address when the line is actually a parameter
		//uint8_t cLineBuf[MAX_SCREEN_SEL][LCD_WIDTH + 1];
	};

	uint8_t cLineBuf1[LCD_WIDTH + 1];
	uint8_t cLineBuf2[LCD_WIDTH + 1];

	uint8_t cNextScreen;
	uint8_t cItemsLimit;
		//for displays < 4 lines
	unsigned int iLatchedStatus;
	//to allow display write to complete
	uint8_t cUpdateViewPhase;	//send all 4 lines to display as necessary

protected:
	uint8_t cPointerPos; // active menu item

	uint8_t cFocusParameter;
	//when the parameter is finally selected, the address is carried on this
	//variable- modified every time the Enter is pressed on a menu selection

	//
	MenuBufferType displayBuffer;
	//
	MenuContext status;

	LiquidCrystal& disp;

protected:
	HorizMenu(LiquidCrystal& lcd);

protected:
	void CreateMenu (uint8_t menuIndex);
	void UpdateView (void);
	void TransferMenu(void);
	//navigation
	void PositionMoveLeft (void);
	void PositionMoveRight (void);
	uint8_t GoToDisplaySelected(void);
	void InvalidAction(void){};
    //
	void ProcessLine (unsigned char LineProcess, unsigned char LineNumber);
	void IncreaseParameter (void);
	void DecreaseParameter (void);

//inner interface
protected:
	void clearPointerPos(void);
	void setPointerPos(uint8_t lineNum){cPointerPos = lineNum;};
	void ResetDisplay();

protected:
	int WriteLine (const uint8_t *bufferStart, uint8_t startLine, uint8_t lineOffset);

};

#endif /* HORIZMENU_H_ */
