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
 main program
 ****************************************************************************/
int main (void) {
    // declare variables

	uint16_t _arPeriods[MAX_PERIODS * 2] = {250,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t  _iPeriod = 0;		// storing current period
	uint8_t  _iTick = 0;		// counting interrupt ticks
	uint16_t _iTime = 0;		// counting time in seconds
	uint16_t _iTemp = 0;		// counting temperature in

	char _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting
	uint8_t   _iButtonOld = 0;	// storing previously pressed button

    // setup registers
    fSetup();

    // default state is halted
    iStatus |= STATUS_RUN;

    // run main program
    while (1) {
	    if (iStatus & STATUS_TICK) {
			_iTick++; // if 10 ticks are passed (iTick reset to 0), 1 second passed
			if (_iTick >= 10) {
				_iTick = 0;
				if (iStatus & STATUS_RUN) {
					_iTime++;
				}
			}
			iStatus &= ~(STATUS_TICK);		// TICK event has been dealt with
	    }
		if (iStatus & STATUS_ADC) {
			if (_iTick == 0) {
				_iTemp = iTempRead / 10;
				iTempRead = 0;
			}
			iStatus &= ~(STATUS_ADC);		// ADC event has been dealt with
		}
	    if (iStatus & STATUS_BUTTON) {
	    	if (iButton != _iButtonOld) {    // new button pressed
				if (iButton & BUTTON_RUN) {
					if (iStatus & STATUS_RUN) {
						iStatus &= ~(STATUS_RUN);
					} else {
						iStatus |= STATUS_RUN;
					}
				}
	    		_iButtonOld = iButton;
	    	}
	    	iStatus &= ~(STATUS_BUTTON);	// BUTTON event has been dealt with
	    }

		if (iStatus & STATUS_RUN) {
			OUT_PORT &= ~(OUT_LED_RED);	    // turn off red led
			OUT_PORT |= OUT_LED_GREEN;	    // turn on green led

			if (_iTime >= _arPeriods[_iPeriod + MAX_PERIODS]) {
				_iPeriod++;
				_iTime = 0;
				if (_arPeriods[_iPeriod + MAX_PERIODS] == 0) {
					iStatus &= ~(STATUS_RUN);
					_iPeriod = 0;
				}
			}
			// turn on the pump
			iStatus |= STATUS_PUMP;

			// if temp is higher than set temp, switch off the heater, switch on cooler
			if (_iTemp > _arPeriods[_iPeriod]) {
				iStatus &= ~(STATUS_HEATER);
				iStatus |= STATUS_COOLER;
			}
			// if temp is lower than set temp, switch on the heater, switch off cooler
			if (_iTemp < _arPeriods[_iPeriod]) {
				iStatus |= STATUS_HEATER;
				iStatus &= ~(STATUS_COOLER);
			}
			// if temp is set temp, switch off the heater, switch off cooler
			if (_iTemp > _arPeriods[_iPeriod]) {
				iStatus &= ~(STATUS_HEATER);
				iStatus &= ~(STATUS_COOLER);
			}
		} else {
			OUT_PORT &= ~(OUT_LED_GREEN);    // turn off green led
			OUT_PORT |= OUT_LED_RED;         // turn on green led
			iStatus &= ~(STATUS_PUMP);
			iStatus &= ~(STATUS_HEATER);
			iStatus &= ~(STATUS_COOLER);
		}

		// update the display
		sprintf(_arLCDline, "  %02d.%02d %02d.%02d P%01x",
			(_arPeriods[_iPeriod] >> 2),
			((_arPeriods[_iPeriod] & 0x0003) * 25),
			_arPeriods[_iPeriod + MAX_PERIODS] / 3600,
			_arPeriods[_iPeriod + MAX_PERIODS] / 60,
			_iPeriod);
		lcd_gotoxy(0, 0); lcd_puts(_arLCDline);
		sprintf(_arLCDline, "  %02d.%02d %02d:%02d:%02d",
			(_iTemp >> 2),
			((_iTemp & 0x0003) * 25),
			(_iTime / 3600),
			(_iTime / 60) % 60,
			(_iTime) % 60);
		lcd_gotoxy(0, 1); lcd_puts(_arLCDline);

		// if pump status = 1, switch on the output else switch off
		if (iStatus & STATUS_PUMP) {
			OUT_PORT |= OUT_PUMP;
		} else {
			OUT_PORT &= ~(OUT_PUMP);
		}
		// if heater status = 1, switch on the output else switch off
		if (iStatus & STATUS_HEATER) {
			OUT_PORT |= OUT_HEATER;
		} else {
			OUT_PORT &= ~(OUT_HEATER);
		}
		// if cooler status = 1, switch on the output else switch off
		if (iStatus & STATUS_COOLER) {
			OUT_PORT |= OUT_COOLER;
		} else {
			OUT_PORT &= ~(OUT_COOLER);
		}
	}

    return 0;
}

/****************************************************************************
 timer1 interrupt routine
 occurs when timer reaches TOP as set in OCR1A
 ****************************************************************************/
ISR(TIMER1_COMPA_vect) {
    KBD_PORT |= BUTTON_LED;         // switch back LED on
    iStatus |= STATUS_TICK;         // set interrupt tick
    iButton = (KBD ^ 0x7f);         // read keyboard
    if (iButton != 0) {
    	iStatus |= STATUS_BUTTON;
    }
    ADCSRA |= _BV(ADSC);            // start capture
}

/****************************************************************************
 analog conversion interrupt routine
 occurs when analog conversion is completed
 ****************************************************************************/
ISR(ADC_vect) {
    iTempRead += ADC; // read ADC
    iStatus |= STATUS_ADC;          // set ADC status to 1
    KBD_PORT &= ~(BUTTON_LED);      // switch back LED off
}
