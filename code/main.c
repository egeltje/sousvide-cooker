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
#include <avr/interrupt.h>
#include "main.h"
#include "config.h"
#include "display.h"
#include "lcd.h"

/****************************************************************************
 main program
 ****************************************************************************/
int main (void) {

	// declare variables
	uint8_t  _iPeriod = 0;		// storing current period
	uint16_t _iTime = 0;		// counting time in seconds
	uint16_t _iTemp = 0;		// current temperature
	uint16_t _iTempStart = 0;	// temperature at start of period
	uint16_t _iTempCalc = 0;	// set temperature at given time
	uint8_t  _iStatus = 0;		// storing system states
	uint8_t  _iButtonOld = 0;	// storing previously pressed button

	// setup registers
    fConfigSetup();

    // Initial update of the display
    lcd_clrscr();
    fDisplayPeriodLine(_iPeriod, 0);

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
				if (iButton & BUTTON_RESET) {
					if (!(_iStatus & STATUS_RUN)) {
						_iTime = 0;
						_iPeriod = 0;
					}
				}
				if (iButton & BUTTON_CONFIG) {
					if (!(_iStatus & STATUS_RUN)) {
						fConfig();
					}
				}
				if (iButton & BUTTON_ARROW_RIGHT) {
					if (!(_iStatus & STATUS_RUN)) {
						if (_iPeriod < (MAX_PERIODS - 1)) {
							_iPeriod++;
						}
					}
				}
				if (iButton & BUTTON_ARROW_LEFT) {
					if (!(_iStatus & STATUS_RUN)) {
						if (_iPeriod > 0) {
							_iPeriod--;
						}
					}
				}
	    		_iButtonOld = iButton;
	    	}
	    }
		if (iTick >= 10) {
			iTick = 0;

			if (_iStatus & STATUS_RUN) _iTime++;

			_iTemp = ((iTempRead / 10) - stCalibration->offset) / stCalibration->coefficient;
			if (_iTemp >= 400) _iTemp = 399;
			iTempRead = 0;
			_iTempCalc = stPeriods[_iPeriod].temp;

			fDisplayTemp(_iTempCalc, 0, 0);

//			_dTempTime = (((stPeriods[_iPeriod].temp - _iTempStart) / stPeriods[_iPeriod].time) * _iTime) + _iTempStart;
		}
		if (_iStatus & STATUS_RUN) {
			OUT_PORT &= ~(OUT_LED_RED);	    // turn off red led
			OUT_PORT |= OUT_LED_GREEN;	    // turn on green led

			if (_iTime >= stPeriods[_iPeriod].time) {
				_iTime = 0;
				_iTempStart = stPeriods[_iPeriod].temp;

				if (stPeriods[_iPeriod].loop) {
					_iPeriod = 0;
				} else {
					_iPeriod++;
					if (_iPeriod > MAX_PERIODS) {
						_iPeriod = 0;
						_iStatus &= ~(STATUS_RUN);
					} else {
						 if (stPeriods[_iPeriod].time == 0) {
							 _iPeriod = 0;
							 _iStatus &= ~(STATUS_RUN);
						 }
					}
				}
			}
			// turn on the pump
			_iStatus |= STATUS_PUMP;

			// if temp is higher than set temp, switch off the heater, switch on cooler
			if (_iTemp > _iTempCalc) {
				_iStatus &= ~(STATUS_HEATER);
				_iStatus |= STATUS_COOLER;
			}
			// if temp is lower than set temp, switch on the heater, switch off cooler
			if (_iTemp < _iTempCalc) {
				_iStatus |= STATUS_HEATER;
				_iStatus &= ~(STATUS_COOLER);
			}
			// if temp is set temp, switch off the heater, switch off cooler
			if (_iTemp == _iTempCalc) {
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
		fDisplayPeriodLine(_iPeriod, 0);
		fDisplayActualLine(_iTemp, _iTime, _iStatus, 1);

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
    iTempRead += ADC;
    KBD_PORT &= ~(BUTTON_LED);      // switch back LED off
}
