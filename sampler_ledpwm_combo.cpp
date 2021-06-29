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
    * - ICR1 = (PWM_OVERFLOW_VALUE * 2) - 1 // 399 // two 10kHz clocks
    * - OCR1A = PWM_OVERFLOW_VALUE - 1 // 199 first 10kHz clock duty stop
    * - OCR1B = PWM_DUTY_CYCLE // 180 // first 10kHz clock duty start (next would be 3039)
    * - TCCR1B |= (1 << CS10) // set prescaler 1
    * enable compA, compB and overflow interrupts in TIMSK
    * - TIMSK |= (1 << OCIE1A) | (1 << OCIE1B) | (1 << TOIE1);
  */  

  cli();
  // Set WGM to Fast PWM
  TCCR1A = 0 | (1 << WGM11)
  TCCR1B = 0 | (1 << WGM13) | (1 << WGM12) | (1 << CS10)

  // set overflow value
  ICR1 = (PWM_OVERFLOW_VALUE * 2) - 1 // 399 // two 10kHz clocks - should be equal to SAMPLER_TIMER_COUNTER_FOR(SAMP_FREQ)
  // TODO: abort if they're different

  // The correct formula here is (F_CPU / ((ICR1+1)*PRESCALER)) - google why
  OCR2A = PWM_OVERFLOW_VALUE - 1; // 199 first 10kHz clock duty stop
  OCR2B = PWM_DUTY_CYCLE;         // 180 // first 10kHz clock duty start (next would be 380)

  // Output Compare Match A & B and Overflow Interrupt Enable
  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B) | (1 << TOIE1);

  sei();
}

/*
Interrupts

We have 4 interrupt points, but only 3 interrupts; so we move COMPB between two values.

          1 2          3 4
|---------B-A------------O|
          ^
compA (high)


|---------B-A------------O|
            ^
compB (low)
move compB to 3
* ld value
* sts OCR1B (change compB to 3)


|-----------A----------B-S|
                       ^
compA (high)



|-----------L----------H-S|
                         ^
compB does low + sampling, compB to 1
* ld value
* sts OCR1B (change compB to 1)

*/