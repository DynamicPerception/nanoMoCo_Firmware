/*
 * ScreenType.h
 *
 *  Created on: Jun 4, 2012
 *      Author: perepelitsa
 */

#ifndef SCREENTYPE_H_
#define SCREENTYPE_H_

#define MAX_SCREEN_SEL 10 // max of menu items per screen

#define LCD_WIDTH 16 // max width of LCD screen
#define LCD_LINES 2

//
struct vector
{
  uint8_t size;
  uint8_t cLine[5][5];
};

//represents one line on screen, including menu header
struct menuItem
{
  uint8_t cMsgNum;      //message to be displayed from resource
  uint8_t cProcNum;     //process to be done on message
  uint8_t cNextType;	//nextItem type (relate to iInterPhase)
  uint8_t cNextScreen;	//what to display when enter is pressed (not used for param entry)
                        //(this can carry a number associated with the line, say EEPROM address for param )
  uint8_t cParamNum;	//parameter associated with line may be *void
  uint8_t cIncr;        //parameter increment by
};

//represents appearance of screen in per-line (items) basis
struct Screen {
uint8_t cNumberOfItems;
uint8_t cArrowsRequired;	//need to display selection arrows
const struct menuItem items[MAX_SCREEN_SEL];
};

#endif /* SCREENTYPE_H_ */
