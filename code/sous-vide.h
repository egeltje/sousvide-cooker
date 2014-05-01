#ifndef SOUSVIDE_H
#define SOUSVIDE_H
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

#define SAMPLE_FREQUENCY	((1000000 / 1024) / 10)
//                          ((F_CPU / CPU_PRESCALER / Hz)
#define TEMP_OFFSET         0       /* for future calibration */

#define STATUS_HALT         0x01    /* status bit for halted operation */
#define STATUS_PUMP         0x02    /* status bit for the pump */
#define STATUS_HEATER       0x04    /* status bit for the heaC 8?ter */
#define STATUS_TIMER        0x08    /* status bit for the timer */
#define STATUS_TIMER_RUN    0x10    /* status bit for the running timer */
#define STATUS_ADC          0x80    /* status bit for the AD converter */

#define KBD_PORT            PORTD   /* the KBD is attached to IO Port D */
#define KBD_DIR             DDRD
#define KBD                 PIND
#define BUTTON_TIMER_SS     0x01    /* pin D0 */
#define BUTTON_TIMER_RST    0x02    /* pin D1 */
#define BUTTON_HALT         0x04    /* pin D2 */
#define BUTTON_ARROW_UP     0x08    /* pin D3 */
#define BUTTON_ARROW_RIGHT  0x10    /* pin D4 */
#define BUTTON_ARROW_DOWN   0x20    /* pin D5 */
#define BUTTON_ARROW_LEFT   0x40    /* pin D6 */

#define OUT_PORT            PORTB   /* the output is attached to IO Port B */
#define OUT_DIR             DDRB
#define OUT_PUMP            0x01    /* pin B0 */
#define OUT_HEATER          0x02    /* pin B1 */
#define OUT_AUX0            0x04    /* pin B2 */
#define OUT_AUX1            0x08    /* pin B3 */
#define OUT_LED0            0x10    /* pin B4 */
#define OUT_LED1            0x20    /* pin B5 */
#define OUT_LED2            0x40    /* pin B6 */
#define OUT_LED3            0x80    /* pin B7 */

#endif //SOUSVIDE_H
