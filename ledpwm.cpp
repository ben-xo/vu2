// AVR Timer CTC Interrupts Calculator
// v. 8
// http://www.arduinoslovakia.eu/application/timer-calculator
// Microcontroller: ATmega328P
// Created: 2019-05-04T13:03:54.561Z

#include <Arduino.h>
#include "config.h"
#include "ledpwm.h"

// when 0, leds should be lit.
uint8_t pwm_duty = PWM_DUTY_CYCLE;
uint8_t portb_val = 0;

void setup_ledpwm() {
  cli();
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  // 10000 Hz (16000000/((24+1)*64))
  OCR2A = 24;
  // CTC
  TCCR2A |= (1 << WGM21);
  // Prescaler 64
  TCCR2B |= (1 << CS22);
  // Output Compare Match A Interrupt Enable
  TIMSK2 |= (1 << OCIE2A);
  sei();
}

ISR(TIMER2_COMPA_vect) {
  if(pwm_duty == 0) {
    PORTB = 0;    
  }
  pwm_duty--;
  if(pwm_duty == 0) {
    PORTB = portb_val;    
  }
  if(pwm_duty == 255) {
    pwm_duty = PWM_DUTY_CYCLE;
  }
}
