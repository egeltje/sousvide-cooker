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

	uint16_t  _arPeriods[MAX_PERIODS] = {250,160,0,0,0,0,0,0,0,0,0,0,0,0,0,0,60,120,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	uint8_t   _iPeriod;

	char _arLCDline[LCD_DISP_LENGTH];      // array for lcd line formatting

	uint8_t  _iTick;
	uint16_t _iTime;
	uint16_t _iTemp;

    // setup registers
    fSetup();

    // default state is halted
//    iStatus |= STATUS_HALT;

    // run main program
    while (1) {
		while (!(iStatus & STATUS_HALT)) {

			if (iStatus & STATUS_TICK) {
				_iTick++;
				// if 10 ticks are passed (iTick reset to 0), 1 second passed
				if (_iTick > 9) {
					_iTick = 0;
				}
				iStatus &= ~(STATUS_TICK);

				if (iStatus & STATUS_TIMER) {
					OUT_PORT &= ~(OUT_LED_RED);	    // turn off red led
					OUT_PORT |= OUT_LED_GREEN;	    // turn on green led
					if (_iTick == 0) {
						_iTime++;
					}
					if (_iTime == _arPeriods[_iPeriod + MAX_PERIODS/ 2]) {
						_iPeriod++;
						if (_arPeriods[_iPeriod + MAX_PERIODS / 2] == 0) {
							iStatus |= STATUS_HALT;
						}
					}
				} else {
					OUT_PORT &= ~(OUT_LED_GREEN);    // turn off green led
					OUT_PORT |= OUT_LED_RED;         // turn on green led
				}

			}
	/*
			if (iButton == BUTTON_TIMER_RUN) {
				if (iStatus & STATUS_TIMER) {
					if (iStatus & STATUS_TIMER_RUN) {
						iStatus &= ~(STATUS_TIMER_RUN);
					} else {
						iStatus |= STATUS_TIMER_RUN;
					}
				}
			}
			if (iButton == BUTTON_TIMER_RST) {
				if (iStatus & STATUS_TIMER) {
					if (iStatus & STATUS_TIMER_RUN) {
					} else {
						iSec = 0;
						iMin = 0;
						iHour = 0;
					}
				}
			}
			if (iButton == BUTTON_HALT) {
				if (iStatus & STATUS_HALT) {
					iStatus &= ~(STATUS_HALT);
				} else {
					iStatus |= STATUS_HALT;
				}
			}

	*/
			if (iStatus & STATUS_ADC) {
				if (iTick==10) {
					iTick = 0;
					iTemp = iTempRead / 10;
					iTempRead = 0;
				}

				if (iStatus & STATUS_HALT) {
					iStatus &= ~(STATUS_PUMP);
					iStatus &= ~(STATUS_HEATER);
					iStatus &= ~(STATUS_TIMER);
				} else {
					// turn on the pump
					iStatus |= STATUS_PUMP;

					// if temp is higher than set temp, switch off the heater
					if (iTemp > iTempSet) {
						iStatus &= ~(STATUS_HEATER);
					}
					// if temp is lower than set temp, switch on the heater
					if (iTemp < iTempSet) {
						iStatus |= STATUS_HEATER;
					}

					// turn on the timer (but do not run it)
					iStatus |= STATUS_TIMER;
				}

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

				// if timer status = 1, record the time

				// update the display
				sprintf(_arLCDline, "Ts%02d.%02d %02d.%02d P%01x",
					(_arPeriods[_iPeriod] >> 2),
					((_arPeriods[_iPeriod] & 0x0003) * 25),
					_arPeriods[_iPeriod + MAX_PERIODS / 2] / 3600,
					_arPeriods[_iPeriod + MAX_PERIODS / 2] / 60,
			_iPeriod);
				lcd_gotoxy(0, 0); lcd_puts(_arLCDline);
				sprintf(_arLCDline, "Tr%02d.%02d %02d:%02d:%02d",
					(iTemp >> 2),
					((iTemp & 0x0003) * 25),
					(_iTime / 3600),
					(_iTime / 60) % 60,
					(_iTime) % 60);
				lcd_gotoxy(0, 1); lcd_puts(_arLCDline);

				// set the ADC status back to 0
				iStatus &= ~(STATUS_ADC);
			}
	  }
    }
  return 0;
}

/****************************************************************************
 timer1 interrupt routine
 occurs when timer reaches TOP as set in OCR1A
 ****************************************************************************/
ISR(TIMER1_COMPA_vect) {
    KBD_PORT |= BUTTON_LED;         // set back LED on
    iStatus |= STATUS_TICK;         // set keyboard status to 1
    iButton = (KBD ^ 0x7f);         // read keyboard
    if (iButton != iButtonOld) {    // new button pressed
        iStatus |= STATUS_BUTTON;   // set keyboard status to 1
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
    KBD_PORT &= ~(BUTTON_LED);      // set back LED off
}

