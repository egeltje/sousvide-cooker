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
#include <stdlib.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "display.h"
#include "lcd.h"
#include "main.h"

/****************************************************************************
 Config routine
 ****************************************************************************/
uint8_t fConfig (void) {
	char *_cMenu[] = {
			"Periods       >",
			"Calibration   >",
			"FACTORY RESET >",
			"Exit          >",
			NULL};
	uint8_t _iExit = 0;

	while (!_iExit) {
		lcd_clrscr();
		fDisplayString("Configuration", 0, DISPLAY_LINE0);	// Current menu

		switch (fConfigMenuChoice(_cMenu)) {
		case 0:
			fConfigPeriods ();
			break;
		case 1:
			fConfigCalibration ();
			break;
		case 2:
			fConfigReset ();
			break;
		case 3:
			_iExit = 1;
			break;
		default:
			_iExit = 1;
			break;
		}
	}

	return 0;
}

uint8_t fConfigCalibration (void) {
	char *_cMenu[] = {
			"0C            >",
			"100C          >",
			"Exit          >",
			NULL};
	uint8_t _iExit = 0;

	while (!_iExit) {
		lcd_clrscr();
		fDisplayString("Calibration", 0, DISPLAY_LINE0);	// Current menu

		switch (fConfigMenuChoice(_cMenu)) {
		case 0:
			lcd_clrscr();
			fDisplayString("Calibration 0C", 0, DISPLAY_LINE0);
			stCalibration->zeroC = fConfigCalibrationMeasurement();
			break;
		case 1:
			lcd_clrscr();
			fDisplayString("Calibration 100C", 0, DISPLAY_LINE0);
			stCalibration->hundredC = fConfigCalibrationMeasurement();
			break;
		case 2:
			_iExit = 1;
			break;
		default:
			_iExit = 1;
			break;
		}
	}

	stCalibration->offset = stCalibration->zeroC;
	stCalibration->coefficient = (stCalibration->hundredC - stCalibration->zeroC) / 100;

	eeprom_write_block((const void*)stCalibration, (void*)sizeof(uint8_t), sizeof(struct calibration));

	return 0;
}

uint16_t fConfigCalibrationMeasurement (void) {
	uint8_t  _iExit = 0;
	uint8_t  _iTemp = 0;
	uint8_t  _iButtonOld = iButton;

	fDisplayString("          Set >", 0, DISPLAY_LINE1);

    while (!_iExit) {
    	if (iButton != _iButtonOld) {    // new button pressed
			if (iButton & BUTTON_ARROW_RIGHT) {
					_iExit = 1;
			}
    		_iButtonOld = iButton;
    	}
		if (iTick >= 10) {
			iTick = 0;

			_iTemp = iTempRead / 10;
			iTempRead = 0;
			fDisplayTemp(_iTemp, 0, DISPLAY_LINE1);
			lcd_gotoxy(14, DISPLAY_LINE1);
		}
    }
    return _iTemp;
}

uint8_t fConfigEEPROM (void) {
	uint8_t _i = 0;
	uint8_t _iEEPROMinit = eeprom_read_byte((uint8_t *)0);

	if (_iEEPROMinit == 0) {
		stCalibration->zeroC = 0;
		stCalibration->hundredC = 399;
		stCalibration->offset = 0;
		stCalibration->coefficient = 1;

		for (_i = 0; _i < MAX_PERIODS; ++_i) {
			stPeriods[_i].temp = 0;
			stPeriods[_i].time = 0;
			stPeriods[_i].loop = 0;
		}
		stPeriods[0].temp = 250;
		stPeriods[0].time = 60;
		stPeriods[0].loop = 0;
		stPeriods[1].temp = 200;
		stPeriods[1].time = 60;
		stPeriods[1].loop = 1;

		eeprom_write_byte((uint8_t *)0, 1);
		eeprom_write_block((const void*)stCalibration, (void*)sizeof(uint8_t), sizeof(struct calibration));
		eeprom_write_block((const void*)stPeriods, (void*)(sizeof(struct calibration) + sizeof(uint8_t)), sizeof(struct periods) * MAX_PERIODS);
	} else {
		eeprom_read_block((void*)stCalibration, (const void*)sizeof(uint8_t), sizeof(struct calibration));
		eeprom_read_block((void*)stPeriods, (const void*)(sizeof(struct calibration) + sizeof(uint8_t)), sizeof(struct periods) * MAX_PERIODS);
	}
	return 0;
}

uint8_t fConfigError (char *pMessage[]) {
	return 0;
}

uint8_t fConfigMenuChoice (char *pMenu[]) {
	uint8_t _iMenuOption = 0;
	uint8_t _iMenuLength = 0;
	uint8_t _iButtonOld  = iButton;
	char _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting
    lcd_command(LCD_DISP_ON_CURSOR);	    // enable display, enable cursor

	while (pMenu[_iMenuLength] != NULL) _iMenuLength++;

	sprintf(_arLCDline, "%s", pMenu[_iMenuOption]);
	lcd_gotoxy(0, 1); lcd_puts(_arLCDline);
	lcd_gotoxy(14, 1);

	while (_iMenuLength > 0) {

		if (iButton != 0) {
			if (iButton != _iButtonOld) {    // new button pressed
				if (iButton & BUTTON_ARROW_RIGHT) {
					_iMenuLength = 0;
				}
				if (iButton & BUTTON_ARROW_UP) {
					if (_iMenuOption == 0) _iMenuOption = _iMenuLength;
					_iMenuOption--;
				}
				if (iButton & BUTTON_ARROW_DOWN) {
					_iMenuOption++;
					if (_iMenuOption >= _iMenuLength) _iMenuOption = 0;
				}
				sprintf(_arLCDline, "%s", pMenu[_iMenuOption]);
				lcd_gotoxy(0, 1); lcd_puts(_arLCDline);
				lcd_gotoxy(14, 1);

				_iButtonOld = iButton;
			}
		}
	}
    lcd_init(LCD_DISP_ON);		    	// enable display, disable cursor

	return _iMenuOption;
}

uint8_t fConfigPeriods () {
	uint8_t _iPeriod = 0;
	char *_cMenu[] = {
			"Next          >",
			"Edit          >",
			"Insert        >",
			"Delete        >",
			"Exit          >",
			NULL};
	uint8_t _iExit = 0;

	while (!_iExit) {

		lcd_clrscr();
		fDisplayPeriodLine(_iPeriod, 0);

		switch (fConfigMenuChoice(_cMenu)) {
		case 0:
			_iPeriod++;
			if ((stPeriods[_iPeriod].time == 0) || (_iPeriod >= MAX_PERIODS)) _iPeriod = 0;
			break;
		case 1:
			fConfigPeriodEdit(_iPeriod);
			break;
		case 2:
			fConfigPeriodAdd(_iPeriod);
			break;
		case 3:
			fConfigPeriodDelete(_iPeriod);
			break;
		case 4:
			_iExit = 1;
			break;
		default:
			_iExit = 1;
			break;
		}
	}

	eeprom_write_block((const void*)stPeriods, (void*)(sizeof(struct calibration) + sizeof(uint8_t)), sizeof(struct periods) * MAX_PERIODS);

	return 0;
}

/****************************************************************************
 Add period routine
 ****************************************************************************/
uint8_t fConfigPeriodAdd (uint8_t iPeriod) {
	uint8_t _iPeriodLast = 0;
	while ((stPeriods[_iPeriodLast].time != 0) && (_iPeriodLast <= MAX_PERIODS)) _iPeriodLast++;

	if (_iPeriodLast < MAX_PERIODS) {
		while (_iPeriodLast > iPeriod) {
			stPeriods[_iPeriodLast].temp = stPeriods[_iPeriodLast - 1].temp;
			stPeriods[_iPeriodLast].time = stPeriods[_iPeriodLast - 1].time;
			stPeriods[_iPeriodLast].loop = stPeriods[_iPeriodLast - 1].loop;
			_iPeriodLast--;
		}

		stPeriods[iPeriod].temp = 0;
		stPeriods[iPeriod].time = 0;
		stPeriods[iPeriod].loop = 0;
		fConfigPeriodEdit(iPeriod);
	}

	return 0;
}

/****************************************************************************
 Delete period routine
 ****************************************************************************/
uint8_t fConfigPeriodDelete (uint8_t iPeriod) {
	while (iPeriod < MAX_PERIODS) {
		stPeriods[iPeriod].temp = stPeriods[iPeriod + 1].temp;
		stPeriods[iPeriod].time = stPeriods[iPeriod + 1].time;
		stPeriods[iPeriod].loop = stPeriods[iPeriod + 1].loop;
		iPeriod++;
	}
	stPeriods[iPeriod].temp = 0;
	stPeriods[iPeriod].time = 0;
	stPeriods[iPeriod].loop = 0;

	return 0;
}

/****************************************************************************
 Edit period routine
 ****************************************************************************/
uint8_t fConfigPeriodEdit(uint8_t iPeriod) {
    uint8_t _iCursorPos = 4;             // storing cursor position
    uint8_t _iButtonOld = iButton;
    uint8_t _iPeriod = iPeriod;
    uint16_t	_iTemp = stPeriods[_iPeriod].temp;
    uint16_t	_iTime = stPeriods[_iPeriod].time;
    uint8_t		_iLoop = stPeriods[_iPeriod].loop;

    lcd_command(LCD_DISP_ON_CURSOR);	    // enable display, enable cursor
    lcd_clrscr();
    lcd_gotoxy(0, DISPLAY_LINE0); lcd_puts("Edit period");
    fDisplayPeriod(_iPeriod, 12, DISPLAY_LINE0);
    fDisplayTemp(_iTemp, 4, DISPLAY_LINE1);
	fDisplayTime(_iTime, 8, DISPLAY_LINE1, DISPLAY_TIME_SHORT);
	fDisplayLoop(_iLoop, 13, DISPLAY_LINE1);
	lcd_gotoxy(14, DISPLAY_LINE1); lcd_putc(0x3e);
	lcd_gotoxy(_iCursorPos, DISPLAY_LINE1);

	while (_iCursorPos < 15) {
		if (iButton != 0) {
			if (iButton != _iButtonOld) {
				if (iButton & BUTTON_ARROW_LEFT) {
					if (_iCursorPos == 5) {		// 10   digit temp
						_iCursorPos = 4;
					}
					if (_iCursorPos == 6) {		//  1   digit temp
						_iCursorPos = 5;
					}
					if (_iCursorPos == 8) {		//   .5 digit temp
						_iCursorPos = 6;
					}
					if (_iCursorPos == 9) {		// 10    digit hour
						_iCursorPos = 8;
					}
					if (_iCursorPos == 11) {	//  1    digit hour
						_iCursorPos = 9;
					}
					if (_iCursorPos == 12) {	//   :10 digit minute
						_iCursorPos = 11;
					}
					if (_iCursorPos == 13) {	//   :01 digit minute
						_iCursorPos = 12;
					}
					if (_iCursorPos == 14) {	//   loop
						_iCursorPos = 13;
					}
				}
				if (iButton & BUTTON_ARROW_RIGHT) {
					if (_iCursorPos == 14) {
						_iCursorPos = 15;
					}
					if (_iCursorPos == 13) {	//   :01 digit minute
						_iCursorPos = 14;
					}
					if (_iCursorPos == 12) {	//   :01 digit minute
						_iCursorPos = 13;
					}
					if (_iCursorPos == 11) {	//   :10 digit minute
						_iCursorPos = 12;
					}
					if (_iCursorPos == 9) {		//  1    digit hour
						_iCursorPos = 11;
					}
					if (_iCursorPos == 8) {		// 10   digit hour
						_iCursorPos = 9;
					}
					if (_iCursorPos == 6) {		//   .5 digit temp
						_iCursorPos = 8;
					}
					if (_iCursorPos == 5) {		//  1   digit temp
						_iCursorPos = 6;
					}
					if (_iCursorPos == 4) {		// 10   digit temp
						_iCursorPos = 5;
					}
				}
				if (iButton & BUTTON_ARROW_DOWN) {
					if (_iCursorPos == 4) {
						if (_iTemp >= 40) _iTemp -= 40;
					}
					if (_iCursorPos == 5) {
						if (_iTemp >= 4) _iTemp -= 4;
					}
					if (_iCursorPos == 6) {
						if (_iTemp >= 2) _iTemp -= 2;
					}
					if (_iCursorPos == 8) {
						if (_iTime >= 36001) _iTime -= 36000;
					}
					if (_iCursorPos == 9) {
						if (_iTime >= 3601) _iTime -= 3600;
					}
					if (_iCursorPos == 11) {
						if (_iTime >= 601) _iTime -= 600;
					}
					if (_iCursorPos == 12) {
						if (_iTime >= 61) _iTime -= 60;
					}
					if (_iCursorPos == 13) {
						_iLoop ^= 1;
					}
				}
				if (iButton & BUTTON_ARROW_UP) {
					if (_iCursorPos == 4) {
						if (_iTemp <= 359) _iTemp += 40;
					}
					if (_iCursorPos == 5) {
						if (_iTemp <= 395) _iTemp += 4;
					}
					if (_iCursorPos == 6) {
						if (_iTemp <= 397) _iTemp += 2;
					}
					if (_iCursorPos == 8) {
						if (_iTime <= 7200) _iTime += 36000;
					}
					if (_iCursorPos == 9) {
						if (_iTime <= 39600) _iTime += 3600;
					}
					if (_iCursorPos == 11) {
						if (_iTime <= 42600) _iTime += 600;
					}
					if (_iCursorPos == 12) {
						if (_iTime <= 42140) _iTime += 60;
					}
					if (_iCursorPos == 13) {
						_iLoop ^= 1;
					}
				}
				_iButtonOld = iButton;
				// update the display
				fDisplayTemp(_iTemp, 4, DISPLAY_LINE1);
				fDisplayTime(_iTime, 8, DISPLAY_LINE1, DISPLAY_TIME_SHORT);
				fDisplayLoop(_iLoop, 13, DISPLAY_LINE1);
				lcd_gotoxy(14, DISPLAY_LINE1); lcd_putc(0x3e);
				lcd_gotoxy(_iCursorPos, DISPLAY_LINE1);
			}
		}
	}

    lcd_command(LCD_DISP_ON);			    // enable display, disable cursor

	stPeriods[_iPeriod].temp = _iTemp;
	stPeriods[_iPeriod].time = _iTime;
	stPeriods[_iPeriod].loop = _iLoop;

	return 0;
}

uint8_t  fConfigReset (void) {
	char *_cMenu[] = {
			"No            >",
			"Yes           >",
			NULL};

	lcd_clrscr();
	lcd_gotoxy(0, 0); lcd_puts("FACTORY RESET");

	switch (fConfigMenuChoice(_cMenu)) {
	case 0:
		break;
	case 1:
		eeprom_write_byte((uint8_t*)0, 0);
		fConfigEEPROM();
		break;
	default:
		break;
	}

	return 0;
}
/****************************************************************************
 config setup routine
 ****************************************************************************/
uint8_t fConfigSetup (void) {
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
    lcd_init(LCD_DISP_ON);			    // enable display
    DDRD |= 0x80;			            // enable pin for background lighting

    // setup custom lcd characters
    static const uint8_t _arCustomChar[64] PROGMEM = {
    	0x00, 0x05, 0x09, 0x1f, 0x08, 0x04, 0x00, 0x00,  // char0 (loop)
    	0x00, 0x00, 0x07, 0x05, 0x05, 0x05, 0x17, 0x00,  // char1 (.0)
       	0x00, 0x00, 0x07, 0x04, 0x07, 0x01, 0x17, 0x00,  // char2 (.5)
    	0x00, 0x0f, 0x12, 0x1d, 0x11, 0x0e, 0x1f, 0x00,  // char3 (pump)
    	0x00, 0x0f, 0x12, 0x1d, 0x11, 0x0e, 0x1f, 0x00,  // char4 (pump)
    	0x00, 0x00, 0x01, 0x02, 0x04, 0x18, 0x18, 0x00,  // char5 (hedgehogL0)
        0x11, 0x02, 0x18, 0x19, 0x02, 0x08, 0x19, 0x00,  // char6 (hedgehogL1)
        0x08, 0x12, 0x00, 0x09, 0x12, 0x00, 0x09, 0x00   // char7 (hedgehogL2)
    };
    lcd_command(_BV(LCD_CGRAM));
    for (_i = 0; _i < 64; _i++) {
        lcd_data(pgm_read_byte(&_arCustomChar[_i]));
    }
    lcd_command(_BV(LCD_CGRAM));

	stCalibration = (struct calibration *)malloc(MAX_CALIBRATION * sizeof(struct calibration));
	stPeriods = (struct periods *)malloc(MAX_PERIODS * sizeof(struct periods));

	fConfigEEPROM();

    sei();                              // enable interrupts

    lcd_gotoxy(0,0); lcd_puts(VERSION);
    lcd_gotoxy(0,1); lcd_puts(VENDOR);
	while (iTick < 25) {};
    LCD_PORT |= 0x80;		            	// turn on background lighting

    return 0;
}
