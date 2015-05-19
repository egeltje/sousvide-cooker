/*
 * display.c
 *
 *  Created on: May 16, 2015
 *      Author: erwin
 */

#include <stdio.h>
#include "display.h"
#include "main.h"
#include "lcd.h"

uint8_t fDisplayActualLine(uint16_t iTemp, uint16_t iTime, uint8_t iStatus, uint8_t iLine) {
	fDisplayTemp(iTemp, 0, iLine);
	fDisplayStatus(iStatus, 4, iLine);
	fDisplayTime(iTime, 8, iLine, DISPLAY_TIME_LONG);
	return 0;
}

uint8_t fDisplayPeriodLine(uint8_t iPeriod, uint8_t iLine) {
	fDisplayTemp(stPeriods[iPeriod].temp, 4, iLine);
	fDisplayTime(stPeriods[iPeriod].time, 8, iLine, DISPLAY_TIME_SHORT);
	fDisplayPeriod(iPeriod, 14, iLine);
	fDisplayLoop(stPeriods[iPeriod].loop, 13, iLine);
	return 0;
}

uint8_t fDisplayTemp(uint16_t iTemp, uint8_t ix, uint8_t iy) {
	char    _arLCDline[3];      // array for lcd line formatting

	sprintf(_arLCDline, "%02d%c",
				(iTemp >> 2),
				((iTemp & 0x0002) >> 1) + 1);
	lcd_gotoxy(ix, iy); lcd_puts(_arLCDline);

	/*	sprintf(_arLCDline, "%02d",
				(iTemp >> 2));
	lcd_gotoxy(ix, iy); lcd_puts(_arLCDline);
	lcd_gotoxy(ix + 2, iy); lcd_putc((iTemp & 0x0002) >> 1);
	);
*/
	return 0;
}

uint8_t fDisplayTime(uint16_t iTime, uint8_t ix, uint8_t iy, uint8_t iFormat) {
	char    _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting

	if (iFormat == 0) {
		sprintf(_arLCDline, "%02d:%02d:%02d",
				(iTime / 3600),
				(iTime / 60) % 60,
				(iTime) % 60);
	} else {
		sprintf(_arLCDline, "%02d:%02d",
						(iTime / 3600),
						(iTime / 60) % 60);
	}
	lcd_gotoxy(ix, iy); lcd_puts(_arLCDline);

	return 0;
}

uint8_t fDisplayLoop(uint16_t iLoop, uint8_t ix, uint8_t iy) {
	if (iLoop) {
		lcd_gotoxy(ix, iy); lcd_putc(0x00);
	} else {
		lcd_gotoxy(ix, iy); lcd_putc(0x20);
	}

	return 0;
}

uint8_t fDisplayPeriod(uint8_t iPeriod, uint8_t ix, uint8_t iy) {
	char    _arLCDline[2];      // array for lcd line formatting
	sprintf(_arLCDline, "%02d",
			iPeriod);
	lcd_gotoxy(ix, iy); lcd_puts(_arLCDline);
	return 0;
}

uint8_t fDisplayStatus(uint8_t iStatus, uint8_t ix, uint8_t iy) {
	if (iStatus & STATUS_PUMP) {
		lcd_gotoxy(ix, iy); lcd_putc(0x50);
	} else {
		lcd_gotoxy(ix, iy); lcd_putc(0x20);
	}
	// if heater status = 1, switch on the output else switch off
	if (iStatus & STATUS_HEATER) {
		lcd_gotoxy(ix + 1, iy); lcd_putc(0x48);
	} else {
		lcd_gotoxy(ix + 1, iy); lcd_putc(0x20);
	}
	// if cooler status = 1, switch on the output else switch off
	if (iStatus & STATUS_COOLER) {
		lcd_gotoxy(ix + 2, iy); lcd_putc(0x43);
	} else {
		lcd_gotoxy(ix + 2, iy); lcd_putc(0x20);
	}
	return 0;
}

uint8_t fDisplayString(char *pDisplayString, uint8_t ix, uint8_t iy) {
	lcd_gotoxy(ix, iy); lcd_puts(pDisplayString);
	return 0;
}
