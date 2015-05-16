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

uint8_t fDisplayPeriod(uint8_t iPeriod, uint8_t iLine) {
	char     _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting

	sprintf(_arLCDline, "  %02d.%02d %02d.%02d %01x",
			(stPeriods[iPeriod].temp >> 2),
			((stPeriods[iPeriod].temp & 0x0003) * 25),
			stPeriods[iPeriod].time / 3600,
			(stPeriods[iPeriod].time / 60) % 60,
			iPeriod);
		lcd_gotoxy(0, iLine); lcd_puts(_arLCDline);
		if (stPeriods[iPeriod].loop) {
			lcd_gotoxy(15, iLine); lcd_putc(0x01);
		} else {
			lcd_gotoxy(15, iLine); lcd_putc(0x20);
		}
		return 0;
}

uint8_t fDisplayActual(uint16_t iTemp, uint16_t iTime, uint8_t iLine) {
	char     _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting

	sprintf(_arLCDline, " %02d.%02d %02d:%02d:%02d",
			(iTemp >> 2),
			((iTemp & 0x0003) * 25),
			(iTime / 3600),
			(iTime / 60) % 60,
			(iTime) % 60);
	lcd_gotoxy(1, 1); lcd_puts(_arLCDline);

	return 0;
}

