/*
 * display.h
 *
 *  Created on: May 16, 2015
 *      Author: erwin
 */

#ifndef DISPLAY_H
#define DISPLAY_H

extern uint8_t fDisplayActualLine(uint16_t iTemp, uint16_t iTime, uint8_t iStatus, uint8_t iLine);
extern uint8_t fDisplayPeriodLine(uint8_t iPeriod, uint8_t iLine);
extern uint8_t fDisplayTemp(uint8_t iTemp, uint8_t ix, uint8_t iy);
extern uint8_t fDisplayTime(uint16_t iTime, uint8_t ix, uint8_t iy, uint8_t iFormat);
extern uint8_t fDisplayLoop(uint16_t iLoop, uint8_t ix, uint8_t iy);
extern uint8_t fDisplayPeriod(uint8_t iPeriod, uint8_t ix, uint8_t iy);
extern uint8_t fDisplayStatus(uint8_t iStatus, uint8_t ix, uint8_t iy);

#endif
