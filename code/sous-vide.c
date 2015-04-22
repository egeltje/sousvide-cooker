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
#include "lcd.h"
#include "sous-vide.h"

volatile uint16_t arPeriods[16][2];
volatile uint16_t iTemp;
volatile uint16_t iTempRead;
volatile uint16_t iTempSet;
volatile uint8_t iHour;
volatile uint8_t iMin;
volatile uint8_t iSec;
volatile uint8_t iTick;
volatile uint8_t iStatus;
volatile uint8_t iPeriod;
uint16_t iTempOffset = 0;           // storing sensor offset

/****************************************************************************
 main program
 ****************************************************************************/
int main (void) {
    // declare variables 
    char lcdline[LCD_DISP_LENGTH];      // array for lcd line formatting 
    uint8_t iCursorPos = 2;             // storing cursor position 
    uint8_t iButton = 0;                // storing pressed button 
    uint8_t iButtonOld = 0;             // storing pressed button 
    uint8_t i;                          // loop variable

    // setup registers 
    cli();                              // disable interrupts 
    KBD_DIR  = 0x80;
    KBD_PORT = 0x7f;                    // enable pull-ups for row inputs 
    LCD_PORT = 0xff;
    DIDR0 = 0x01;			// disconnect ADC0 (PORTC.1) from IO
    OUT_DIR = 0x3e;                     // enable PORTC pins for output 
    TCCR1B = _BV(WGM12) |               // CTC mode, top = OCR1A 
             _BV(CS10) | _BV(CS12);     // 1024 prescale 
    TIMSK1 = _BV(OCIE1A);                // enable timer1 interrupt 
    OCR1A = SAMPLE_FREQUENCY;           // top is calculated to be 1 sec
    ADCSRA = _BV(ADIE) |                // enable adc interrupt 
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	// 128 prescale 
    ADMUX = _BV(REFS1) | _BV(REFS0);    // internal 2.56V reference 
    ACSR  = _BV(ACD);                   // disable analog comparator 
    ADCSRA |= _BV(ADEN);            // enable ADC 
    sei();                              // enable interrupts 

    lcd_init(LCD_DISP_ON_CURSOR);       // enable display 
    DDRD |= 0x80;			// enable pin for background ligting
    PORTD |= 0x80;			// turn on background lighting

    // setup custom lcd characters
    static const uint8_t arCustomChar[64] PROGMEM = {
        0x04, 0x0a, 0x0a, 0x0e, 0x1f, 0x1f, 0x0e, 0x00,  // char0 (temp)
        0x00, 0x0f, 0x12, 0x1d, 0x11, 0x0e, 0x1f, 0x00,  // char1 (pump)
        0x02, 0x09, 0x00, 0x12, 0x09, 0x00, 0x12, 0x00,  // char2 (hedgehogR0)
        0x11, 0x08, 0x03, 0x13, 0x08, 0x02, 0x03, 0x00,  // char3 (hedgehogR1)
        0x00, 0x00, 0x10, 0x08, 0x04, 0x03, 0x03, 0x00,  // char4 (hedgehogR2)
        0x00, 0x00, 0x01, 0x02, 0x04, 0x18, 0x18, 0x00,  // char5 (hedgehogL0)
        0x11, 0x02, 0x18, 0x19, 0x02, 0x08, 0x19, 0x00,  // char6 (hedgehogL1)
        0x08, 0x12, 0x00, 0x09, 0x12, 0x00, 0x09, 0x00   // char7 (hedgehogL2)
    };
    lcd_command(_BV(LCD_CGRAM));
    for (i = 0; i < 64; i++) {
        lcd_data(pgm_read_byte(&arCustomChar[i]));
    }
    lcd_command(_BV(LCD_CGRAM));

    iTempOffset = eeprom_read_word((uint16_t*)0);
//    eeprom_read_block((void*)&arPeriods, (const void*)2, 32);

    iStatus |= STATUS_HALT;
    iPeriod = iTempOffset;

    // run main program 
    while (1) {

        if (iStatus & STATUS_ADC) {
            iTick++;
            if (iTick==10) {
                iTick = 0;
                iTemp = iTempRead / 10;
                iTempRead = 0;
            }

            iButton = (KBD ^ 0x7f);
            if (iButton != iButtonOld) {    // new button pressed (debounce) 
                // BUTTON_ARROW_LEFT and BUTTON_ARROW_RIGHT move the cursor 
                // position: pos2=decimals, pos3=single digits, pos5=fraction
                // (pos 4 is decimal point on the display) 
                // BUTTON_ARROW_UP and BUTTON_ARROW_DOWN change the value of 
                // the number the cursor is at. There are 4 steps in a single
                // degree. 
                if (iButton == BUTTON_ARROW_LEFT) {
                    if (iCursorPos == 3) {
                        iCursorPos = 2;
                    }
                    if (iCursorPos == 5) {
                        iCursorPos = 3;
                    }
                }
                if (iButton == BUTTON_ARROW_DOWN) {
                    if (iCursorPos == 2) {
                        if (iTempSet >= 40) iTempSet -= 40;
                    }
                    if (iCursorPos == 3) {
                        if (iTempSet >= 4) iTempSet -= 4;
                    }
                    if (iCursorPos == 5) {
                        if (iTempSet >= 1) iTempSet -= 1;
                    }
                }
                if (iButton == BUTTON_ARROW_UP) {
                    if (iCursorPos == 2) {
                        if (iTempSet <= 359) iTempSet += 40;
                    }
                    if (iCursorPos == 3) {
                        if (iTempSet <= 395) iTempSet += 4;
                    }
                    if (iCursorPos == 5) {
                        if (iTempSet <= 398) iTempSet += 1;
                    }
                }
                if (iButton == BUTTON_ARROW_RIGHT) {
                    if (iCursorPos == 3) {
                        iCursorPos = 5;
                    }
                    if (iCursorPos == 2) {
                        iCursorPos = 3;
                    }
                }
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
                
                iButtonOld = iButton;   // the button has been processed 
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
            
            // if timer status = 1 and timer_run status = 1, record the time
            if ((iStatus & STATUS_TIMER) && (iStatus & STATUS_TIMER_RUN)) {
                OUT_PORT &= ~(OUT_LED_RED);	    // turn off red led
                OUT_PORT |= OUT_LED_GREEN;	    // turn on green led
                // if 10 ticks are passed (iTick reset to 0), 1 second passed
                if (iTick == 0) {
                    iSec++;
                    if (iSec > 59) {
                        iSec = 0;
                        iMin++;
                        if (iMin > 59) {
                            iMin = 0;
                            iHour++;
                            if (iHour > 99) {
                                iHour = 0;
                            }
                        }
                    }
                }
            } else {
                OUT_PORT &= ~(OUT_LED_GREEN);    // turn off green led
                OUT_PORT |= OUT_LED_RED;         // turn on green led
            }

            // update the display
            sprintf(lcdline, "Ts%02d.%02d 00:00 P%01x",
                (iTempSet >> 2),
                ((iTempSet & 0x0003) * 25),
		iPeriod);
            lcd_gotoxy(0, 0); lcd_puts(lcdline);
            sprintf(lcdline, "Tr%02d.%02d %02d:%02d:%02d",
                (iTemp >> 2),
                ((iTemp & 0x0003) * 25),
                iHour,
                iMin,
                iSec);
            lcd_gotoxy(0, 1); lcd_puts(lcdline);

            // set the ADC status back to 0
            iStatus &= ~(STATUS_ADC);
        }
  }
  return 0;
}

/****************************************************************************
 timer1 interrupt routine
 occurs when timer reaches TOP as set in OCR1A
 ****************************************************************************/
ISR(TIMER1_COMPA_vect) {
    PORTB |= 0x80;                  // set LED on
    ADCSRA |= _BV(ADSC);            // start capture 
}

/****************************************************************************
 analog conversion interrupt routine
 occurs when analog conversion is completed
 ****************************************************************************/
ISR(ADC_vect) {
    PORTB &= ~(0x80);               // set LED off
    iTempRead += ADC - iTempOffset; // read ADC 
    iStatus |= STATUS_ADC;          // set ADC status to 1
}

