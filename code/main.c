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
	struct calibration _stCalibration;

	struct periods _stPeriods[MAX_PERIODS];
	uint8_t  _iPeriod = 0;		// storing current period

_stPeriods[0].temp = 250;
_stPeriods[0].time = 60;
_stPeriods[1].temp = 16;
_stPeriods[1].time = 120;

	uint16_t _iTime = 0;		// counting time in seconds
	uint16_t _iTemp = 0;		// counting temperature in
	uint8_t  _iStatus;			// storing system states

	char _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting
	uint8_t   _iButtonOld = 0;	// storing previously pressed button

    // setup registers
    fSetup();

    // run main program
    while (1) {
	    if (iButton != 0) {
	    	if (iButton != _iButtonOld) {    // new button pressed
				if (iButton & BUTTON_RUN) {
					if (_iStatus & STATUS_RUN) {
						_iStatus &= ~(STATUS_RUN);
					} else {
						_iStatus |= STATUS_RUN;
					}
				}
				if (iButton & BUTTON_CONFIG) {
					if (!(_iStatus & STATUS_RUN)) {
						fConfig(&_stPeriods, &_stCalibration);
					}
				}
	    		_iButtonOld = iButton;
	    	}
	    }
		if (iTick >= 10) {
			iTick = 0;

			_iTemp = iTempRead / 10;
			iTempRead = 0;

			if (_iStatus & STATUS_RUN) {
				_iTime++;
				OUT_PORT &= ~(OUT_LED_RED);	    // turn off red led
				OUT_PORT |= OUT_LED_GREEN;	    // turn on green led

				if (_iTime >= _stPeriods[_iPeriod].time) {
					_iPeriod++;
					_iTime = 0;
					if (_stPeriods[_iPeriod].time == 0) {
						_iStatus &= ~(STATUS_RUN);
						_iPeriod = 0;
					}
				}
				// turn on the pump
				_iStatus |= STATUS_PUMP;

				// if temp is higher than set temp, switch off the heater, switch on cooler
				if (_iTemp > _stPeriods[_iPeriod].temp) {
					_iStatus &= ~(STATUS_HEATER);
					_iStatus |= STATUS_COOLER;
				}
				// if temp is lower than set temp, switch on the heater, switch off cooler
				if (_iTemp < _stPeriods[_iPeriod].temp) {
					_iStatus |= STATUS_HEATER;
					_iStatus &= ~(STATUS_COOLER);
				}
				// if temp is set temp, switch off the heater, switch off cooler
				if (_iTemp == _stPeriods[_iPeriod].temp) {
					_iStatus &= ~(STATUS_HEATER);
					_iStatus &= ~(STATUS_COOLER);
				}
			} else {
				OUT_PORT &= ~(OUT_LED_GREEN);    // turn off green led
				OUT_PORT |= OUT_LED_RED;         // turn on green led
				_iStatus &= ~(STATUS_PUMP);
				_iStatus &= ~(STATUS_HEATER);
				_iStatus &= ~(STATUS_COOLER);
			}
			// update the display
			sprintf(_arLCDline, "  %02d.%02d %02d.%02d P%01x",
				(_stPeriods[_iPeriod].temp >> 2),
				((_stPeriods[_iPeriod].temp & 0x0003) * 25),
				_stPeriods[_iPeriod].time / 3600,
				_stPeriods[_iPeriod].time / 60,
				_iPeriod);
			lcd_gotoxy(0, 0); lcd_puts(_arLCDline);
			sprintf(_arLCDline, "  %02d.%02d %02d:%02d:%02d",
				(_iTemp >> 2),
				((_iTemp & 0x0003) * 25),
				(_iTime / 3600),
				(_iTime / 60) % 60,
				(_iTime) % 60);
			lcd_gotoxy(0, 1); lcd_puts(_arLCDline);
		}

		// if pump status = 1, switch on the output else switch off
		if (_iStatus & STATUS_PUMP) {
			OUT_PORT |= OUT_PUMP;
		} else {
			OUT_PORT &= ~(OUT_PUMP);
		}
		// if heater status = 1, switch on the output else switch off
		if (_iStatus & STATUS_HEATER) {
			OUT_PORT |= OUT_HEATER;
		} else {
			OUT_PORT &= ~(OUT_HEATER);
		}
		// if cooler status = 1, switch on the output else switch off
		if (_iStatus & STATUS_COOLER) {
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
    iTick++;				        // add interrupt tick
    iButton = (KBD ^ 0x7f);         // read keyboard
    ADCSRA |= _BV(ADSC);            // start capture
}

/****************************************************************************
 analog conversion interrupt routine
 occurs when analog conversion is completed
 ****************************************************************************/
ISR(ADC_vect) {
    iTempRead += ADC; // read ADC
    KBD_PORT &= ~(BUTTON_LED);      // switch back LED off
}
