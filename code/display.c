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
	fDisplayTemp(iTemp, 0, 1);
	fDisplayStatus(iStatus, 4, 1);
	fDisplayTime(iTime, 8, 1, 0);
	return 0;
}

uint8_t fDisplayPeriodLine(uint8_t iPeriod, uint8_t iLine) {
	fDisplayTemp(stPeriods[iPeriod].temp, 4, 0);
	fDisplayTime(stPeriods[iPeriod].time, 8, 0, 1);
	fDisplayPeriod(iPeriod, 14, 0);
	fDisplayLoop(stPeriods[iPeriod].loop, 15, 0);
	return 0;
}

uint8_t fDisplayTemp(uint8_t iTemp, uint8_t ix, uint8_t iy) {
	char    _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting
	char	_cHalf = 0x20;

	if ((iTemp & 0x0002) >> 1)  _cHalf = 0x01;

	sprintf(_arLCDline, "%02d%c",
				(iTemp >> 2),
				_cHalf);
	lcd_gotoxy(ix, iy); lcd_puts(_arLCDline);

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
	lcd_gotoxy(ix, iy); lcd_putc(0x30 + iPeriod);
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

