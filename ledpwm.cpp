// AVR Timer CTC Interrupts Calculator
// v. 8
// http://www.arduinoslovakia.eu/application/timer-calculator
// Microcontroller: ATmega328P
// Created: 2019-05-04T13:03:54.561Z

#include <Arduino.h>
#include "config.h"
#include "ledpwm.h"

// when 0, leds should be lit.
uint8_t volatile portb_val = 0;

void setup_ledpwm() {
  cli();
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;

  // 10000 Hz (16000000/((24+1)*64)) - 64 is in enable_pwm()
  OCR2A = 24; // 24 for 10kHz
  OCR2B = 22; // duty cycle 10% of OCR2A
  
  // CTC
  TCCR2A = (1 << WGM21);
  
  // Output Compare Match A & B Interrupt Enable
  // TIMER2_COMPA_vect clears the LEDs, TIMER2_COMPB_vect lights them.
  TIMSK2 |= (1 << OCIE2A) | (1 << OCIE2B); 

  // this clears the timer and sets the right pre-scaler, starting the timer.
  enable_ledpwm();
  sei();  
}

/**
 * Called before going into a context when the interrupts are just a waste of time 
 * (such as rendering pixels on the light strip)
 */
void disable_ledpwm() {
  // disable the timer entirely, then make sure the lights are off.
  TCCR2B = 0;
  PORTB = 0;
}

void enable_ledpwm() {
  // re-enable the timer (with pre-scaler 64);
  TCNT2 = 0;
  TCCR2B = (1 << CS22);
}

//ISR(TIMER2_COMPA_vect) {
//  PORTB = 0;
//}

ISR(TIMER2_COMPA_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t");
  asm volatile( "ldi     r24, 0                          \n\t"); // ldi doesn't affect SREG
  asm volatile( "out     %0, r24     ; PORTB             \n\t" :: "I" (_SFR_IO_ADDR(PORTB)));
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}
//
//ISR(TIMER2_COMPB_vect) {
//  PORTB = portb_val;
//}

ISR(TIMER2_COMPB_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t");
  asm volatile( "lds     r24, %0     ; portb_val         \n\t" :: "X" ((uint8_t)_SFR_MEM_ADDR(portb_val)));
  asm volatile( "out     %0, r24     ; PORTB             \n\t" :: "I" (_SFR_IO_ADDR(PORTB)));
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}
