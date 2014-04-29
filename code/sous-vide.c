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
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "lcd.h"
#include "sous-vide.h"

volatile uint16_t iTemp;
volatile uint16_t iTempRead;
volatile uint16_t iTempSet;
volatile uint8_t iHour;
volatile uint8_t iMin;
volatile uint8_t iSec;
volatile uint8_t iTick;
volatile uint8_t iStatus;

/****************************************************************************
 main program
 ****************************************************************************/
int main (void) {
    // declare variables 
    char lcdline[LCD_DISP_LENGTH];      // array for lcd line formatting 
    uint8_t iCursorPos = 2;             // storing cursor position 
    uint8_t iButton = 0;                // storing pressed button 
    uint8_t iButtonOld = 0;             // storing pressed button 

    // setup registers 
    cli();                              // disable interrupts 
    KBD_PORT = 0xff;                    // enable pull-ups for row inputs 
    OUT_DIR = 0xff;                     // enable all pins for output 
    TCCR1B = _BV(WGM12) |               // CTC mode, top = OCR1A 
             _BV(CS10) | _BV(CS12);     // 1024 prescale 
    TIMSK = _BV(OCIE1A);                // enable timer1 interrupt 
    OCR1A = SAMPLE_FREQUENCY;           // top is calculated to be 1 sec 
    ADCSRA = _BV(ADIE) |                // enable adc interrupt 
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	// 128 prescale 
    ADMUX = _BV(REFS1) | _BV(REFS0);    // internal 2.56V reference 
    ACSR  = _BV(ACD);                   // disable analog comparator 
    sei();                              // enable interrupts 

    lcd_init(LCD_DISP_ON_CURSOR);       // enable display 

   // setup custom lcd characters 
    static const uint8_t cgstring[16] PROGMEM = {
        0x04, 0x0a, 0x0a, 0x0e, 0x1f, 0x1f, 0x0e, 0x00,  // char 0 (temp) 
        0x00, 0x0f, 0x12, 0x1d, 0x11, 0x0e, 0x1f, 0x00 // char 1 (pump) 
    };
    lcd_command(_BV(LCD_CGRAM));
    uint8_t i;                          // loop variable 
	for (i = 0; i < 16; i++) {
       lcd_data(pgm_read_byte(&cgstring[i]));
	}
	lcd_command(_BV(LCD_CGRAM));

    iStatus |= _BV(STATUS_HALT);
    
    // run main program 
    while (1) {
        if (iStatus & _BV(STATUS_ADC)) {
            iTick++;
            if (iTick==10) {
                iTick = 0;
                iTemp = iTempRead / 10;
                iTempRead = 0;
            }

            iButton = (KBD ^ 0xff);
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
                if (iButton == BUTTON_TIMER_SS) {
                    if (iStatus & _BV(STATUS_TIMER)) {
                        iStatus &= ~(_BV(STATUS_TIMER));
                    } else {
                        iStatus |= _BV(STATUS_TIMER);
                    }
                }
                if (iButton == BUTTON_TIMER_RST) {
                    if (iStatus & _BV(STATUS_TIMER)) {
                    } else {
                        iSec = 0;
                        iMin = 0;
                        iHour = 0;
                    }
                }
                if (iButton == BUTTON_HALT) {
                    if (iStatus & _BV(STATUS_HALT)) {
                        iStatus &= ~(_BV(STATUS_HALT));
                    } else {
                        iStatus |= _BV(STATUS_HALT);
                    }
                }
                iButtonOld = iButton;   // the button has been processed 
            }

            if (iStatus & _BV(STATUS_HALT)) {
                OUT_PORT |= _BV(OUT_LED3);  // turn on led as warning

                iStatus &= ~(_BV(STATUS_PUMP));
                iStatus &= ~(_BV(STATUS_HEATER));
            } else {
                OUT_PORT &= ~(_BV(OUT_LED3));   // turn off led

                iStatus |= _BV(STATUS_PUMP);    // turn on pump 
                // if temp is higher than set temp, switch off the heater
                if (iTemp > iTempSet) {
                    iStatus &= ~(_BV(STATUS_HEATER));
                }
                // if temp is lower than set temp, switch on the heater
                if (iTemp < iTempSet) {
                    iStatus |= _BV(STATUS_HEATER);
                }
                // if timer status = 1, record the elapsed time
                if (iStatus & _BV(STATUS_TIMER)) {
                    OUT_PORT |= _BV(OUT_LED1);	// turn on led as warning
                    // if 10 ticks are passed (iTick reset to 0) 1 has second passed
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
                    OUT_PORT &= ~(_BV(OUT_LED1));
                }
            }

            // if pump status = 1, switch on the output else switch off
            if (iStatus & _BV(STATUS_PUMP) && ~(iStatus & _BV(STATUS_HALT))) {
                OUT_PORT |= _BV(OUT_PUMP);      // turn on pump 
            } else {
                OUT_PORT &= ~(_BV(OUT_PUMP));   // turn off pump 
            }
           
            // if heater status = 1, switch on the output else switch off
            if (iStatus & _BV(STATUS_HEATER)) {
                OUT_PORT |= _BV(OUT_HEATER);
            } else {
                OUT_PORT &= ~(_BV(OUT_HEATER));
            }
            

            
            // update the display
            sprintf(lcdline, "Ts%02d.%02d  Tr%02d.%02d",
                (iTempSet >> 2),
                ((iTempSet & 0x0003) * 25),
                (iTemp >> 2),
                ((iTemp & 0x0003) * 25));
            lcd_gotoxy(0, 0); lcd_puts(lcdline);
            sprintf(lcdline, "[ ] %02d:%02d:%02d [ ]",
                iHour,
                iMin,
                iSec);
            lcd_gotoxy(0, 1); lcd_puts(lcdline);

            if (iStatus & _BV(STATUS_HEATER)) {
                lcd_gotoxy(1, 1); lcd_putc(0x00);
            };
        
            if (iStatus & _BV(STATUS_PUMP)) {
                lcd_gotoxy(14, 1); lcd_putc(0x01);
            };
            
            lcd_gotoxy(iCursorPos, 0);

            // set the ADC status back to 0
            iStatus &= ~_BV(STATUS_ADC);
        }
    }
    return 0;
}

/****************************************************************************
 timer1 interrupt routine
 occurs when timer reaches TOP as set in OCR1A
 ****************************************************************************/
ISR(TIMER1_COMPA_vect) {
    ADCSRA |= _BV(ADEN);            // enable ADC 
    OUT_PORT |= _BV(OUT_LED2);      // set LED on 
    ADCSRA |= _BV(ADSC);            // start capture 
}

/****************************************************************************
 analog conversion interrupt routine
 occurs when analog conversion is completed
 ****************************************************************************/
ISR(ADC_vect) {
    OUT_PORT &= ~(_BV(OUT_LED2));   // set LED off 
    iTempRead += ADC - TEMP_OFFSET; // read ADC 
    iStatus |= _BV(STATUS_ADC);     // set ADC status to 1 
    ADCSRA &= ~(_BV(ADEN));         // disable ADC 
}

