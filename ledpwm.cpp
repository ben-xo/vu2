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
uint8_t volatile portb_val = 0;

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

//ISR(TIMER2_COMPA_vect) {
//  if(pwm_duty == 0) {
//    pwm_duty = PWM_DUTY_CYCLE;
//    PORTB = portb_val;
//  }
//  else {
//    pwm_duty--;   
//    PORTB = 0;
//  }
//}


// This version should take 16 cycles off and 18 cycles on.
ISR(TIMER2_COMPA_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t");
  asm volatile( "in      r24, __SREG__                   \n\t");
  asm volatile( "push    r24                             \n\t");
  asm volatile( "lds     r24, %0     ; pwm_duty          \n\t" :: "X" ((uint8_t)_SFR_MEM_ADDR(pwm_duty)));
  asm volatile( "cpi     r24, 0x00                       \n\t");
  asm volatile( "breq    .+10                            \n\t");
  asm volatile( "subi    r24, 0x01   ; 1                 \n\t");
  asm volatile( "sts     %0, r24     ; pwm_duty          \n\t" :: "X" ((uint8_t)_SFR_MEM_ADDR(pwm_duty)));
  asm volatile( "eor     r24,r24                         \n\t");
  asm volatile( "rjmp    .+10                            \n\t");
  asm volatile( "ldi     r24, %0     ; PWM_DUTY_CYCLE    \n\t" :: "M" (PWM_DUTY_CYCLE));
  asm volatile( "sts     %0, r24     ; pwm_duty          \n\t" :: "X" ((uint8_t)_SFR_MEM_ADDR(pwm_duty)));
  asm volatile( "lds     r24, %0     ; portb_val         \n\t" :: "X" ((uint8_t)_SFR_MEM_ADDR(portb_val)));
  asm volatile( "out     %0, r24     ; PORTB             \n\t" :: "I" (_SFR_IO_ADDR(PORTB)));
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "out     __SREG__, r24                   \n\t");
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}
