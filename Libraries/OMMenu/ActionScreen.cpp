/*
 * ActionScreen.cpp
 *
 *  Created on: 05.10.2012
 *      Author: perepelitsa
 */
#include <string.h>
#include "ActionScreen.h"

/**
 *
 * */
ActionScreen::ActionScreen(MenuContext & _status)
: status(_status)
{
	memset(actions, 0, sizeof(actionCB) * ACTIONS_MAX);
	action = 0;
	markerPos = 0;
	moveDir = 0;
}

/**
 *
 **/
void ActionScreen::openAction(const uint8_t idxProc, const uint8_t idxParam)
{
  status.openParameter(idxParam);
  if (idxProc < ACTIONS_MAX) {
    action = actions[idxProc];
  } else {
    action = 0;
  }
}

/**
 *
 **/
void ActionScreen::closeAction(bool closeFlag)
{
  status.closeParameter(closeFlag);
  if (closeFlag) {
	  if (action) {
		  //ToDo
		  action(2);
	  }
  }
  action = 0;
}

/**
 *
 **/
void ActionScreen::Left(void)
{
	//uint32_t param = status.readLiveParameter(curIdx);
	markerPos--;
	markerPos = (markerPos & 0x0F);
	moveDir = 0;
	if (action) {
	  action(moveDir);
	}
	status.setContextBits(TRANSFER_DISPLAY_ENABLE);
}

/**
 *
 **/
void ActionScreen::Right(void)
{
	//uint32_t param = status.readLiveParameter(curIdx);
	markerPos++;
	markerPos = (markerPos & 0x0F);
	moveDir = 1;
	if (action) {
		action(moveDir);
	}
	status.setContextBits(TRANSFER_DISPLAY_ENABLE);
}


