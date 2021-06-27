/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// AVR Timer CTC Interrupts Calculator
// v. 8
// http://www.arduinoslovakia.eu/application/timer-calculator
// Microcontroller: ATmega328P
// Created: 2019-05-04T13:03:54.561Z

#include <Arduino.h>
#include "config.h"
#include "ledpwm.h"

#define PWM_PRESCALER 8 // must match what enable_ledpwm() does

// e.g. at 16MHz, overflow val will be 200. At 20Mhz, 250. At 8MHz, 100.
#define PWM_OVERFLOW_VALUE (F_CPU / PWM_LED_FRQ / PWM_PRESCALER)

// e.g. at 16MHz, duty val will be 180. At 20Mhz, 225. At 8MHz, 90.
#define PWM_DUTY_VALUE     (PWM_OVERFLOW_VALUE - (PWM_OVERFLOW_VALUE / (100 / PWM_DUTY_PERCENT)))

/* definition to expand macro then apply to pragma message */
/* from https://stackoverflow.com/questions/1562074/how-do-i-show-the-value-of-a-define-at-compile-time */
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

// compile time debug to see the PWM vals
#pragma message(VAR_NAME_VALUE(PWM_OVERFLOW_VALUE))
#pragma message(VAR_NAME_VALUE(PWM_DUTY_VALUE))

uint8_t volatile portb_val = 0;

static DigitalPin<BEAT_PIN_1> beat_pin;
static DigitalPin<BEAT_PIN_2> tempo_pin;

void setup_ledpwm() {
#ifndef LEDPWM_NO_ISRS
  cli();
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;

  // The correct formula here is (F_CPU / ((OCR2A+1)*PRESCALER)) - google why
  OCR2A = PWM_OVERFLOW_VALUE - 1; // 24 for 10kHz with prescaler 64, 199 for 10kHz with prescaler 8
  OCR2B = PWM_DUTY_VALUE;         // duty cycle 10% of OCR2A
  
  // CTC
  TCCR2A = (1 << WGM21);
  
  // Output Compare Match A & B Interrupt Enable
  // TIMER2_COMPA_vect clears the LEDs, TIMER2_COMPB_vect lights them.
  TIMSK2 |= (1 << OCIE2A) | (1 << OCIE2B); 

  // this clears the timer and sets the right pre-scaler, starting the timer.
  enable_ledpwm();
  sei();
#endif /* LEDPWM_NO_ISRS */
}

#ifndef LEDPWM_NO_ISRS

void disable_ledpwm() {
  // disable the timer entirely, then make sure the lights are off.
  TCCR2B = 0;
  PORTB = 0;
}

void enable_ledpwm() {
  // start at an offset so that PWM interrupts don't coincide with Sampler interrupts
  // Without this, sometimes we get unstable PWM when interrupts pile up.
  TCNT2 = 30;
  
  TCCR2B = (1 << CS21); // re-enable the timer (with pre-scaler 8) - change PWM_PRESCALER if you change this
//  TCCR2B = (1 << CS22); // re-enable the timer (with pre-scaler 64) - change PWM_PRESCALER if you change this
}

//ISR(TIMER2_COMPA_vect) {
//  PORTB = 0;
//}

ISR(TIMER2_COMPA_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t");
  asm volatile( "ldi     r24, 0                          \n\t"); // ldi doesn't affect SREG
  asm volatile( "out     %0, r24     ; PORTB             \n\t" :: "I" (_SFR_IO_ADDR(PORTB)));
  beat_pin.low();
  tempo_pin.low();
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}
//
//ISR(TIMER2_COMPB_vect) {
//  PORTB = portb_val;
//}
//


ISR(TIMER2_COMPB_vect, ISR_NAKED) {
  asm volatile( "push    r1                              \n\t");
  asm volatile( "push    r24                             \n\t");
  asm volatile( "ldi     r24, 0                          \n\t"); // loading 0 into r24 and then copying it to r1 means we don't have to push and pop the SREG.
  asm volatile( "mov     r1, r24                         \n\t");
  asm volatile( "lds     r24, %0     ; portb_val         \n\t" :: "X" ((uint8_t)_SFR_MEM_ADDR(portb_val)));
  asm volatile( "out     %0, r24     ; PORTB             \n\t" :: "I" (_SFR_IO_ADDR(PORTB)));

  register bool is_beat_1 asm ("r24") = F.is_beat_1;
  if(is_beat_1) beat_pin.high(); // this compiles to a `cpse` which doesn't affect the S reg!

  register bool is_beat_2 asm ("r24") = F.is_beat_2;
  if(is_beat_2) tempo_pin.high(); // this compiles to a `cpse` which doesn't affect the S reg!
  
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "pop     r1                              \n\t");
  asm volatile( "reti                                    \n\t");
}

#endif /* LEDPWM_NO_ISRS */
