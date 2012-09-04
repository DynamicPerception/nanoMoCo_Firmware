/*
 * HorizMenu.cpp
 *
 *  Created on: 04.09.2012
 *      Author: perepelitsa
 */

#include "HorizMenu.h"

HorizMenu::HorizMenu(LiquidCrystal& lcd)
 : disp(lcd)
{
	cUpdateViewPhase = 0;
}

/**
 * Reset display.
 * Called from child class to reset display.
 * */
void HorizMenu::ResetDisplay(){
	disp.begin(LCD_WIDTH, LCD_LINES);
    Resources::readGlyph(0,cLineBuf2);
	disp.createChar(LEFT_ARROW_GLYPH,cLineBuf2);
	Resources::readGlyph(1,cLineBuf2);
	disp.createChar(RIGHT_ARROW_GLYPH,cLineBuf2);
    disp.clear();
    disp.home();
}

/**
 * Update the display based on several requests in status.
 * Called by framework.
 * Updates one line per call for smooth look.
 * */
void HorizMenu::UpdateView (void)
{
	switch (cUpdateViewPhase)
	{
		case 0:
		//check if any update is needed
		//including the flashing of the colon in the time display
			iLatchedStatus = status.getContext();
			status.clrContextBits(WRITE_LINE1 | WRITE_LINE2);
			//so a request is not lost when this process is writing a line
			//and a new request is made.
			if ((iLatchedStatus & (WRITE_LINE1 | WRITE_LINE2)) ||
			     ((status.getContext() & (TIME_DISABLE | COLON_REQUEST)) == COLON_REQUEST)) {
				cUpdateViewPhase++;
			}
	    break;
		case 1:
			if (iLatchedStatus & WRITE_LINE1)
			{
				WriteLine(cLineBuf1, 0, 0);
				iLatchedStatus &= ~WRITE_LINE1;
			}
			cUpdateViewPhase++;
		break;
		case 2:
			if (iLatchedStatus & WRITE_LINE2)
			{
				WriteLine(cLineBuf2, 1, 0);
				iLatchedStatus &= ~WRITE_LINE2;
				status.clrContextBits(COLON_REQUEST);
			}
			cUpdateViewPhase = 0;

		break;

		default:
			cUpdateViewPhase=0;
			break;
	}
}


/**
 * Copy from buffer to frame buffer with offset.
 * Draw arrows.
 * Set update flags for all lines.
 *
 * */
void HorizMenu::TransferMenu(void)
{//starting at the top line
  if (status.getContext() & TRANSFER_DISPLAY_ENABLE)
	{
		status.clrContextBits(TRANSFER_DISPLAY_ENABLE);
		//clear so doesn't continuously execute
		//every time it is called

		//copy the associated message across PGM space
		uint8_t msgNum = Resources::readMsgNum(displayBuffer.cCurrentScreen, cPointerPos);
		Resources::readMsgToBuf(msgNum, cLineBuf2);

		cLineBuf1[LCD_WIDTH] = 0;
		cLineBuf2[LCD_WIDTH] = 0;

		//
		if (displayBuffer.cArrowsRequired)
		{
			//left - right arrows
			if (cItemsLimit > LCD_LINES)
			{
				if (cPointerPos != 0)
				{
					cLineBuf2[0] = LEFT_ARROW_GLYPH;

				}
				if (cPointerPos < cItemsLimit)
				{
					cLineBuf2[LCD_WIDTH - 1] = RIGHT_ARROW_GLYPH;
				}
			}
		}
		status.setContextBits(WRITE_LINE1 | WRITE_LINE2 );
	}
}


/***
 *
 **/
void HorizMenu::CreateMenu (uint8_t In_msgNum)
{
	// clearing the parameter type- 0xff
	//indicates that the line is to be blanked
	for (unsigned char i=0; i < MAX_SCREEN_SEL; i++) {
		displayBuffer.cParamNum[i] = UNUSED_PARAMETER;
		//set up for unused line
	}
	//transferring all relevant information from the screen
	//to the display buffer
	displayBuffer.cArrowsRequired = Resources::readArrowsReq(In_msgNum);
	displayBuffer.cCurrentScreen = In_msgNum;
    //uint8_t msgNum[4] = {0,1,2,3};
	uint8_t cActualNumberOfItems = Resources::readNumberOfItems(In_msgNum);
	for (uint8_t i=0; i < cActualNumberOfItems; i++)
	{//do for number of lines in the particular display
		displayBuffer.cNextAction[i] = Resources::readNextType(In_msgNum,i);
		//so we know what to do next
		displayBuffer.cAssociatedDisplay[i] = Resources::readNextScreen(In_msgNum,i);
		//so we know what and display
		displayBuffer.cParamNum[i] = Resources::readParamNum(In_msgNum,i);
		//save the parameter
	}

	//copy the 0th message across PGM space as menu header
	uint8_t msgNum = Resources::readMsgNum(In_msgNum, 0);
	Resources::readMsgToBuf(msgNum, cLineBuf1);

	//do not count header
	cItemsLimit = cActualNumberOfItems - 1;

	//and force update when transfer message next seen
	status.setContextBits(TRANSFER_DISPLAY_ENABLE);
}


/**
 * Navigate menu items up direction
 * 0th element is header
 * */
void HorizMenu::PositionMoveLeft (void)
{
	if (cPointerPos > 1)
		cPointerPos--;

	status.setContextBits(TRANSFER_DISPLAY_ENABLE);
}

/**
 * Navigate menu items down direction
 * */
void HorizMenu::PositionMoveRight (void)
{
	if ((cPointerPos < (cItemsLimit-1)) &&
		(displayBuffer.cParamNum[cPointerPos+1] != UNUSED_PARAMETER)) {
		cPointerPos++;
	}
	status.setContextBits(TRANSFER_DISPLAY_ENABLE);
}

/**
 * Open menu item.
 * */
uint8_t HorizMenu::GoToDisplaySelected(void)
{
    unsigned int phase = displayBuffer.cNextAction[cPointerPos];
	//in this case all lines point to the same target
	cFocusParameter = displayBuffer.cParamNum[cPointerPos];
	//take the parameter to use later.
	//if this is not a parameter entry, then no harm is done.
	status.openParameter(cFocusParameter);
	cNextScreen = displayBuffer.cAssociatedDisplay[cPointerPos];
	CreateMenu(cNextScreen);
	clearPointerPos();
	//must be done separately to allow correct display creation
	return phase;
}

/**
 * Sets initial positions of frame and pointer in new menu screen.
 * */
void HorizMenu::clearPointerPos(void)
{
	//must be done separately to allow correct display creation
	cPointerPos = 1;
	status.setContextBits(TIME_DISABLE);
	//prevent any further time update at deeper levels
}

/**
 * Writes line from buffer to given position on LCD
 * buffer is zero terminated
 * May use String class
 *
 * */
int HorizMenu::WriteLine (const uint8_t *bufferStart, uint8_t startLine, uint8_t lineOffset)
{
 disp.setCursor(lineOffset, startLine);
 disp.print((const char*)bufferStart);
 return 0;
}
