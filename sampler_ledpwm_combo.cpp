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
#include "sampler_ledpwm_combo.h"

void setup_sampler_ledpwm_combo() {

  setup_sampler(0); // needed to set up the ADC, which we aren't messing with in this file

  /*
    Set up the following:
    * timer1 mode to FastPWM w/ICR1 for TOP val
    * - set WGM13, WGM12, WGM11, WGM10 = 1 1 1 0
    * - TCCR1A = 0 | (1 << WGM11)
    * - TCCR1B = 0 | (1 << WGM13) | (1 << WGM12)
    * timer1 to 10KHz with prescaler 1 
    * The correct formula here is (F_CPU / ((ICR1+1)*PRESCALER)) - google why
    * - ICR1 = 3199 // two 10kHz clocks
    * - OCR1A = 1599 // first 10kHz clock duty stop
    * - OCR1B = 1439 // first 10kHz clock duty start (next would be 3039)
    * - TCCR1B |= (1 << CS10)
    * enable compA, compB and overflow interrupts in TIMSK
    * - TIMSK |= (1 << OCIE1A) | (1 << OCIE1B) | (1 << TOIE1);
  */  

  cli();
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;

  // The correct formula here is (F_CPU / ((ICR1+1)*PRESCALER)) - google why
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
}