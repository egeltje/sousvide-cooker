#ifndef CONFIG_H_
#define CONFIG_H_
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

extern uint8_t  fConfig (void);
extern uint8_t  fConfigCalibration (void);
extern uint16_t fConfigCalibrationMeasurement (void);
extern uint8_t  fConfigEEPROM (void);
extern uint8_t  fConfigError (char *pMessage[]);
extern uint8_t  fConfigMenuChoice (char *pMenu[]);
extern uint8_t  fConfigPeriods (void);
extern uint8_t  fConfigPeriodAdd (uint8_t iPeriod);
extern uint8_t  fConfigPeriodDelete (uint8_t iPeriod);
extern uint8_t  fConfigPeriodEdit (uint8_t iPeriod);
extern uint8_t  fConfigReset (void);
extern uint8_t  fConfigSetup (void);

#endif /* CONFIG_H_ */
