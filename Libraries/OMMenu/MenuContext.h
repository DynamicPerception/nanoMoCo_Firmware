/*
 * MenuContext.h
 *
 *  Created on: Jun 4, 2012
 *      Author: perepelitsa
 */
#include <inttypes.h>

#ifndef MENUCONTEXT_H_
#define MENUCONTEXT_H_

#define WRITE_LINE1 0x0001		/*request write to line 1*/
#define WRITE_LINE2 0x0002		/*request write to line 2*/
#define WRITE_LINE3 0x0004		/*request write to line 3*/
#define WRITE_LINE4 0x0008		/*request write to line 4*/
#define TIME_DISABLE 0x0010	    /*disable time update on screen */
#define COLON_REQUEST 0x0020	/*write colon to screen */
#define MINUTES_CHANGED 0x0040	/*in order to update time on display */

#define PB_DELTA 0x0080	         /*valid pushbutton detected */
#define KEYBOARD_VALID 0x0100	 /* keyboard available for analysis */
#define TRANSFER_DISPLAY_ENABLE 0x0200	/* enable TransferMessage procedure */

#define UNUSED_PARAMETER 0xff
//unused to allow detection of unused display line

#define ACTIVITY_TIME 5	//every 500mS


#define NUMBER_OF_TIMERS 3
#define T_ACTIVITY 0	//flash colon
#define T_KEYBOARD_REPEAT 1 //timer for auto repeat of keyboard
#define T_KEYBOARD_TIMEOUT 2 //timeout of keyboard

#define NUMBER_OF_PARAMETERS 32

#define INVALID_IDX 0xFF




/**
 * Contains timers and most important stateful  variables.
 * Support parameter edit protocol.
 * */
class MenuContext {


	uint16_t iParamValue[NUMBER_OF_PARAMETERS]; //parameters value array to edit
	//
	const static unsigned int iParamMaxValue[NUMBER_OF_PARAMETERS];
	const static unsigned int iParamMinValue[NUMBER_OF_PARAMETERS];
	const static unsigned int iStepParameters[NUMBER_OF_PARAMETERS];

	char dynList[6][17];

    //ToDo define sizes
	const static char fixedList1[3][7];//None,Camera,Bulb
    const static char fixedList2[2][12];//Begin,StartOver
	const static char fixedList3[2][5];//Yes,No
	const static char fixedList4[8][3];//
	const static char fixedList5[2][17];//

	uint16_t periodTimers[NUMBER_OF_TIMERS];

	uint8_t iKeyboardIncrement;
	uint8_t cFocusParameter;
	uint16_t iModifiableValue;
	uint16_t iStatus;
		//status byte with the following bit allocations

		//parameter stored to allow parameter to be modified
		//or cancelled by just pressing menu and not enter
    uint8_t keyCode;

protected:
    uint8_t checkParamRange(uint8_t idx, uint16_t* pParam);
public:
	MenuContext();

//context interface
	unsigned int getContext() const {return iStatus;}
	void setContextBits(uint16_t mask) { iStatus |= mask;}
	void clrContextBits(uint16_t mask) { iStatus &= ~(mask);}
//timer interface
	unsigned int getTimer(uint8_t idx)const {return periodTimers[idx];}
	void setTimer(uint8_t idx, unsigned int value);
	void decTimer(uint8_t idx);

//keyboard interface
	uint8_t getKeyboardCode() const {return keyCode;};
    void setKeyboardCode(uint8_t);

//parameter interface
    bool isParamEdit();
	void openParameter(uint8_t idx);
	void incParameter();
	void decParameter();
	unsigned int closeParameter(bool saveFlag);
	/* return last position in string*/
	uint8_t formatParameterText(uint8_t idx, uint8_t* cLineBuf);
};

#endif /* MENUCONTEXT_H_ */
