/* BSD 2-Clause license:
 *
 * Copyright (c) 2013, Erwin Kooi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "main.h"
#include "lcd.h"
#include "config.h"

/****************************************************************************
 Config routine
 ****************************************************************************/
uint8_t fConfig (struct periods *stPeriods, struct calibration *stCalibration) {

	const char *_cMenu[] = {"Exit", "Periods", "Calibration", NULL};

	lcd_gotoxy(0, 0); lcd_puts("Configuration   ");

	switch (fConfigMenuChoice(_cMenu)) {
		case 0:
			break;
		case 1:
			fConfigPeriods (stPeriods);
			break;
		case 2:
			fConfigCalibration (stCalibration);
			break;
	}

	return 0;
}

uint8_t fConfigCalibration (struct calibration *stCalibration) {

	const char *_cMenu[] = {"Exit", "Steam", "Ice", NULL};

	lcd_gotoxy(0, 0); lcd_puts("Calibration     ");

	switch (fConfigMenuChoice(_cMenu)) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
	}
	return 0;
}

uint8_t fConfigPeriods (struct periods *stPeriods) {
	return 0;
}

uint8_t fConfigMenuChoice (const char *pMenu[]) {

	uint8_t _iMenuOption = 0;
	uint8_t _iButtonOld  = 0;
	uint8_t _iMenuLength = 0;
	char _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting

	while (pMenu[_iMenuLength] != NULL) _iMenuLength++;

	lcd_gotoxy(0, 1); lcd_puts("                ");

	while (_iMenuLength > 0) {
		sprintf(_arLCDline, "%s", pMenu[_iMenuOption]);
		lcd_gotoxy(0, 1); lcd_puts(_arLCDline);

		if (iButton != 0) {
			if (iButton != _iButtonOld) {    // new button pressed
				if (iButton & BUTTON_ARROW_RIGHT) {
					_iMenuLength = 0;
				}
				if (iButton & BUTTON_ARROW_UP) {
					_iMenuOption++;
					if (_iMenuOption >= _iMenuLength) _iMenuOption = 0;
				}
				if (iButton & BUTTON_ARROW_DOWN) {
					if (_iMenuOption == 0) _iMenuOption = _iMenuLength;
					_iMenuOption--;
				}
				_iButtonOld = iButton;
			}
		}
	}

	return _iMenuOption;
}


/****************************************************************************
 Add period routine
 ****************************************************************************/
uint8_t fConfigPeriodAdd (struct periods *stPeriods, uint8_t iPeriod) {

    if (iPeriod < MAX_PERIODS) {
        fConfigPeriodEdit(stPeriods, iPeriod++);
    } else {
        return 1;
    }
    return 0;
}

/****************************************************************************
 Edit period routine
 ****************************************************************************/
uint8_t fConfigPeriodEdit(struct periods *stPeriods, uint8_t iPeriod) {

    uint16_t _iPeriodTemp = stPeriods[iPeriod].temp;
    uint16_t _iPeriodTime = stPeriods[iPeriod].time;

    char _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting
    uint8_t _iCursorPos = 2;             // storing cursor position
    uint8_t _iButtonOld = 0;

	while (_iCursorPos <= 16) {
		if (iButton != 0) {
			if (iButton != _iButtonOld) {
				// BUTTON_ARROW_LEFT and BUTTON_ARROW_RIGHT move the cursor
				// position: pos2=decimals, pos3=single digits, pos5=fraction
				// (pos 4 is decimal point on the display)
				// BUTTON_ARROW_UP and BUTTON_ARROW_DOWN change the value of
				// the number the cursor is at. There are 4 steps in a single
				// degree.
				if (iButton & BUTTON_ARROW_LEFT) {
					if (_iCursorPos == 3) {      // 10    digit temp
						_iCursorPos = 2;
					}
					if (_iCursorPos == 5) {      //  1    digit temp
						_iCursorPos = 3;
					}
					if (_iCursorPos == 9) {      //   .25 digit temp
						_iCursorPos = 5;
					}
					if (_iCursorPos == 10) {     // 10    digit hour
						_iCursorPos = 9;
					}
					if (_iCursorPos == 12) {     //  1    digit hour
						_iCursorPos = 10;
					}
					if (_iCursorPos == 13) {     //   :10 digit minute
						_iCursorPos = 12;
					}
					if (_iCursorPos == 16) {     //   :01 digit minute
						_iCursorPos = 13;
					}
				}
				if (iButton & BUTTON_ARROW_RIGHT) {
					if (_iCursorPos == 16) {
						_iCursorPos = 17;
					}
					if (_iCursorPos == 13) {     //   :01 digit minute
						_iCursorPos = 16;
					}
					if (_iCursorPos == 12) {     //   :10 digit minute
						_iCursorPos = 13;
					}
					if (_iCursorPos == 10) {     //  1    digit hour
						_iCursorPos = 12;
					}
					if (_iCursorPos == 9) {      // 10    digit hour
						_iCursorPos = 10;
					}
					if (_iCursorPos == 5) {      //   .25 digit temp
						_iCursorPos = 9;
					}
					if (_iCursorPos == 3) {      //  1    digit temp
						_iCursorPos = 5;
					}
					if (_iCursorPos == 2) {      // 10    digit temp
						_iCursorPos = 3;
					}
				}
				if (iButton & BUTTON_ARROW_DOWN) {
					if (_iCursorPos == 2) {
						if (_iPeriodTemp >= 40) _iPeriodTemp -= 40;
					}
					if (_iCursorPos == 3) {
						if (_iPeriodTemp >= 4) _iPeriodTemp -= 4;
					}
					if (_iCursorPos == 5) {
						if (_iPeriodTemp >= 1) _iPeriodTemp -= 1;
					}
					if (_iCursorPos == 9) {
						if (_iPeriodTime >= 36001) _iPeriodTime -= 36000;
					}
					if (_iCursorPos == 10) {
						if (_iPeriodTime >= 3601) _iPeriodTime -= 3600;
					}
					if (_iCursorPos == 12) {
						if (_iPeriodTime >= 601) _iPeriodTime -= 600;
					}
					if (_iCursorPos == 13) {
						if (_iPeriodTime >= 61) _iPeriodTime -= 60;
					}
				}
				if (iButton & BUTTON_ARROW_UP) {
					if (_iCursorPos == 2) {
						if (_iPeriodTemp <= 359) _iPeriodTemp += 40;
					}
					if (_iCursorPos == 3) {
						if (_iPeriodTemp <= 395) _iPeriodTemp += 4;
					}
					if (_iCursorPos == 5) {
						if (_iPeriodTemp <= 398) _iPeriodTemp += 1;
					}
					if (_iCursorPos == 9) {
						if (_iPeriodTime <= 7200) _iPeriodTime += 36000;
					}
					if (_iCursorPos == 10) {
						if (_iPeriodTime <= 39600) _iPeriodTime += 3600;
					}
					if (_iCursorPos == 12) {
						if (_iPeriodTime <= 42600) _iPeriodTime += 600;
					}
					if (_iCursorPos == 13) {
						if (_iPeriodTime <= 42140) _iPeriodTime += 60;
					}
				}
				_iButtonOld = iButton;
			}

			// update the display
			sprintf(_arLCDline, "  %02d.%02d %02d:%02d  >",
				(_iPeriodTemp >> 2),
				((_iPeriodTemp & 0x0003) * 25),
				(_iPeriodTime/3600),
				(_iPeriodTime/60) % 60);
			lcd_gotoxy(0, 1); lcd_puts(_arLCDline);
		}
	}

    stPeriods[iPeriod].temp = _iPeriodTemp;
    stPeriods[iPeriod].time = _iPeriodTime;

	return 0;
}

/****************************************************************************
 config setup routine
 ****************************************************************************/
uint8_t fConfigSetup (struct periods *stPeriods, struct calibration *stCalibration) {
    uint8_t _i;                         // loop variable

    cli();                              // disable interrupts

    // setup keyboard
    KBD_DIR  = 0x80;                    // enable keyboard pins for input
    KBD_PORT = 0x7f;                    // enable pull-ups for row inputs

    // setup output
    OUT_DIR = 0x3e;                     // enable PORTC pins for output
    DIDR0 = 0x01;			            // disconnect ADC0 (PORTC.1) from IO

    // setup timer
    TCCR1B = _BV(WGM12) |               // CTC mode, top = OCR1A
             _BV(CS10) | _BV(CS12);     // 1024 prescale
    TIMSK1 = _BV(OCIE1A);               // enable timer1 interrupt
    OCR1A = SAMPLE_FREQUENCY;           // top is calculated to be 1 sec

    // setup ADC
    ADCSRA = _BV(ADIE) |                // enable adc interrupt
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	// 128 prescale
    ADMUX = _BV(REFS1) | _BV(REFS0);    // internal 2.56V reference
    ACSR  = _BV(ACD);                   // disable analog comparator
    ADCSRA |= _BV(ADEN);                // enable ADC

    // setup display
    lcd_init(LCD_DISP_ON_CURSOR);       // enable display
    DDRD |= 0x80;			            // enable pin for background lighting
    PORTD |= 0x80;		            	// turn on background lighting

    // setup custom lcd characters
    static const uint8_t arCustomChar[64] PROGMEM = {
        0x00, 0x0f, 0x12, 0x1d, 0x11, 0x0e, 0x1f, 0x00,  // char0 (pump)
    	0x00, 0x05, 0x09, 0x1f, 0x08, 0x04, 0x00, 0x00,  // char1 (loop)
        0x02, 0x09, 0x00, 0x12, 0x09, 0x00, 0x12, 0x00,  // char2 (hedgehogR0)
        0x11, 0x08, 0x03, 0x13, 0x08, 0x02, 0x03, 0x00,  // char3 (hedgehogR1)
        0x00, 0x00, 0x10, 0x08, 0x04, 0x03, 0x03, 0x00,  // char4 (hedgehogR2)
        0x00, 0x00, 0x01, 0x02, 0x04, 0x18, 0x18, 0x00,  // char5 (hedgehogL0)
        0x11, 0x02, 0x18, 0x19, 0x02, 0x08, 0x19, 0x00,  // char6 (hedgehogL1)
        0x08, 0x12, 0x00, 0x09, 0x12, 0x00, 0x09, 0x00   // char7 (hedgehogL2)
    };
    lcd_command(_BV(LCD_CGRAM));
    for (_i = 0; _i < 64; _i++) {
        lcd_data(pgm_read_byte(&arCustomChar[_i]));
    }
    lcd_command(_BV(LCD_CGRAM));

	eeprom_read_block((void*)stCalibration, (const void*)0, sizeof(struct calibration));
	eeprom_read_block((void*)stPeriods, (const void*)sizeof(struct calibration), sizeof(struct periods) * MAX_PERIODS);
/*
//  DUMMY VALUES FOR TESTING
	stPeriods[0].temp = 250;
	stPeriods[0].time = 60;
	stPeriods[0].loop = 0;
	stPeriods[1].temp = 160;
	stPeriods[1].time = 120;
	stPeriods[1].loop = 1;
	stPeriods[2].temp = 0;
	stPeriods[2].time = 0;
	stPeriods[2].loop = 0;
//	eeprom_write_block((const void*)stCalibration, (void*)0, sizeof(struct calibration));
//	eeprom_write_block((const void*)stPeriods, (void*)sizeof(struct calibration), sizeof(struct periods) * MAX_PERIODS);
*/
    sei();                              // enable interrupts

    return 0;
}
