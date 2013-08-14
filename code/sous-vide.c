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
#include <avr/sleep.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include "lcd.c"

#define LEDPORT     PORTE
#define LED_ON      6
#define DISPLAY_FREQUENCY	((16000000 / 1024) / 10)
//                          ((F_CPU / CPU_PRESCALER / Hz)
#define TEMP_OFFSET 0

#define STATUS_PUMP     0
#define STATUS_HEATER   1
#define STATUS_TIMER    2
#define STATUS_ADC      3
#define STATUS_KBD      0xf0

#define KBD_PORT        PORTB
#define KBD             PINB
#define BUTTON_ARROW_UP     0x04
#define BUTTON_ARROW_RIGHT  0x01
#define BUTTON_ARROW_DOWN   0x02
#define BUTTON_ARROW_LEFT   0x08
#define BUTTON_TIMER_SS     0x10
#define BUTTON_TIMER_RST    0x20
#define BUTTON_PUMP         0x40

#define OUT_PORT        PORTC
#define OUT_DIR         DDRC
#define OUT_PUMP        6
#define OUT_HEATER      7

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
    /* declare variables */
    uint8_t i;                          /* loop variable */
    char lcdline[LCD_DISP_LENGTH];      /* array for lcd line formatting */
    uint8_t iCursorPos = 2;             /* storing cursor position */
    uint8_t iButton = 0;                    /* storing pressed button */
    uint8_t iButtonOld = 0;                    /* storing pressed button */
    
    /* setup registers */
    cli();                              /* disable interrupts */
    set_sleep_mode(SLEEP_MODE_IDLE);    /* set sleep mode */
    DDRE  = _BV(LED_ON);                /* enable pins for data output */
    KBD_PORT = 0xff;           /* enable pull-ups for row inputs */
    OUT_DIR = 0xff;             /* enable all pins for output */
    TCCR1B = _BV(WGM12) |               /* CTC mode, top = OCR1A */
             _BV(CS10) | _BV(CS12);     /* 1024 prescale */
    TIMSK1 = _BV(OCIE1A);               /* enable timer1 interrupt */
    OCR1A = DISPLAY_FREQUENCY;          /* top is calculated to be 1 second */
    ADCSRA = _BV(ADIE) |                /* enable adc interrupt */
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);	/* 128 prescale */
    ADMUX = _BV(REFS1) | _BV(REFS0);    /* internal 2.56V reference */
    ACSR  = _BV(ACD);                   /* disable analog comparator */
    lcd_init(LCD_DISP_ON_CURSOR);       /* enable display */
    sei();                              /* enable interrupts */

    /* setup custom lcd characters */
    static const uint8_t cgstring[16] PROGMEM = {
        0x00, 0x0f, 0x12, 0x1d, 0x11, 0x0e, 0x1f, 0x00, /* char index 0 */
        0x04, 0x0a, 0x0a, 0x0e, 0x1f, 0x1f, 0x0e, 0x00  /* char index 1 */
    };
    lcd_command(_BV(LCD_CGRAM));
	for (i = 0; i < 16; i++) {
        lcd_data(pgm_read_byte(&cgstring[i]));
	}
	lcd_command(_BV(LCD_CGRAM));
    
    iStatus |= _BV(STATUS_PUMP);
    
	/* run main program */
    while (1) {
        if (iStatus & _BV(STATUS_ADC)) {
            
            iButton = (KBD ^ 0xff);
            if (iButton != iButtonOld) {
                if (iButton == BUTTON_ARROW_RIGHT) { 
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
                if (iButton == BUTTON_ARROW_LEFT) {
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
                if (iButton == BUTTON_PUMP) {
                    if (iStatus & _BV(STATUS_PUMP)) {
                        iStatus &= ~(_BV(STATUS_PUMP));
                    } else {
                        iStatus |= _BV(STATUS_PUMP);
                    }
                }
            
                iButtonOld = iButton;
            }
            
            if (iStatus & _BV(STATUS_PUMP)) {
                OUT_PORT |= _BV(OUT_PUMP);
            } else {
                OUT_PORT &= ~(_BV(OUT_PUMP));
            }
            
            if ((iStatus & _BV(STATUS_HEATER)) && (iTempRead > iTempSet)) {
                OUT_PORT &= ~(_BV(OUT_HEATER));
                iStatus &= ~(_BV(STATUS_HEATER));
            }
            if (~(iStatus & _BV(STATUS_HEATER)) && (iTempRead < iTempSet)) {
                OUT_PORT |= _BV(OUT_HEATER);
                iStatus |= _BV(STATUS_HEATER);
            }

            if (iStatus & _BV(STATUS_TIMER)) {
                iTick++;
                if (iTick == 10) {
                    iTick = 0;
                    iSec++;
                    if (iSec > 59) {
                        iSec = 0;
                        iMin++;
                        if (iMin > 59) {
                            iMin = 0;
                            iHour++;
                            if (iHour > 99) {
                                iHour = 99;
                            }
                        }
                    }
                }
            }
            
            sprintf(lcdline, "Ts%02d.%02d  Tr%02d.%02d",
                (iTempSet >> 2),
                ((iTempSet & 0x0003) * 25),
                (iTempRead >> 2),
                ((iTempRead & 0x0003) * 25));
            lcd_gotoxy(0, 0); lcd_puts(lcdline);
            sprintf(lcdline, "[ ] %02d:%02d:%02d [ ]",
                iHour,
                iMin,
                iSec);
            lcd_gotoxy(0, 1); lcd_puts(lcdline);

            if (iStatus & _BV(STATUS_PUMP)) {
                lcd_gotoxy(1, 1); lcd_putc(0x00);
            };
        
            if (iStatus & _BV(STATUS_HEATER)) {
                lcd_gotoxy(14, 1); lcd_putc(0x01);
            };
            
            lcd_gotoxy(iCursorPos, 0);

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
    ADCSRA |= _BV(ADEN);            /* enable ADC */
    LEDPORT |= _BV(LED_ON);         /* set LED on */
    ADCSRA |= _BV(ADSC);            /* start capture */
}

/****************************************************************************
 analog conversion interrupt routine
 occurs when analog conversion is completed
 ****************************************************************************/
ISR(ADC_vect) {
    LEDPORT &= ~(_BV(LED_ON));      /* set LED off */
    iTempRead = ADC - TEMP_OFFSET;  /* read ADC */
    iStatus |= _BV(STATUS_ADC);     /* set ADC reaady status */
    ADCSRA &= ~(_BV(ADEN));         /* disable ADC */
}

