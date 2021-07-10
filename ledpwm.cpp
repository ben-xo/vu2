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

// we're attaching the FPS calculation to the ledpwm interrupt so lower the number of interrupts.
#include "fps.h"

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

// uint8_t volatile portb_val = 0;

static DigitalPin<BEAT_PIN_1> beat_pin;
static DigitalPin<BEAT_PIN_2> tempo_pin;

void __inline__ fps_count()
{

  static int8_t fps_interrupt_count = PWM_LED_FRQ / FPS;
  volatile uint8_t* fic = &fps_interrupt_count; 
  // // using an intermediate variable makes the compiled output much more efficient.
  // int8_t new_interrupt_count = fps_interrupt_count - 1;
  // if(!new_interrupt_count) {
  //   GPIOR0 |= (1<<1);
  //   new_interrupt_count = interrupt_reset_val;
  // }
  // fps_interrupt_count = new_interrupt_count;

  asm volatile( 
    "lds r24, %[fic]            \n\t" 
    "subi  r24, 0x01            \n\t" 
    "brne  .+4            \n\t" 
    "sbi %0, 1            \n\t" 
    "ldi r24, %1        \n\t" 
    "sts %[fic], r24        \n\t" 
    : 
    : 
    "I" (_SFR_IO_ADDR(GPIOR0)),
    "M" (INTERRUPT_RESET_VAL),
    [fic] "i" (fic)

  );

  // // using an intermediate variable makes the compiled output much more efficient.
  // int8_t new_interrupt_count = fps_interrupt_count - 1;
  //   156c: 80 91 00 01   lds r24, 0x0100 ; 0x800100 <fps_interrupt_count>
  //   1570: 81 50         subi  r24, 0x01 ; 1
  // if(!new_interrupt_count) {
  //   1572: 11 f4         brne  .+4       ; 0x1578 <__vector_7+0x1a>
  //   GPIOR0 |= (1<<1);
  //   1574: f1 9a         sbi 0x1e, 1 ; 30
  //   new_interrupt_count = interrupt_reset_val;
  //   1576: 82 e4         ldi r24, 0x42 ; 66
  // }
  // fps_interrupt_count = new_interrupt_count;
  //   1578: 80 93 00 01   sts 0x0100, r24 ; 0x800100 <fps_interrupt_count>  
}

void setup_ledpwm() {
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
}

void disable_ledpwm() {
  // disable the timer entirely, then make sure the lights are off.
  TCCR2B = 0;
  PORTB = 0;
}

void enable_ledpwm() {
  // start at an offset so that PWM interrupts don't coincide with Sampler interrupts
  // Without this, sometimes we get unstable PWM when interrupts pile up.
  TCNT2 = 70;
  
  TCCR2B = (1 << CS21); // re-enable the timer (with pre-scaler 8) - change PWM_PRESCALER if you change this
//  TCCR2B = (1 << CS22); // re-enable the timer (with pre-scaler 64) - change PWM_PRESCALER if you change this
}


ISR(TIMER2_COMPA_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t");

  // "PORTB = 0" would set PORTB from r1, but we can't guarantee that's 0.
  // ldi rN, 0 doesn't affect SREG, but we can't ldi into r1 (has to be r15+)
  // so, do it manually

  asm volatile( "ldi     r24, 0                          \n\t"); 
  asm volatile( "out     %0, r24     ; PORTB             \n\t" :: "I" (_SFR_IO_ADDR(PORTB)));

  beat_pin.low();
  tempo_pin.low();

  // unfortunately we need to backup SREG for fps_count
  asm volatile( "push    r25                             \n\t");
  asm volatile( "in      r25, %0                         \n\t" :: "I" (_SFR_IO_ADDR(SREG)));
  fps_count();
  asm volatile( "out     %0, r25                         \n\t" :: "I" (_SFR_IO_ADDR(SREG)));
  asm volatile( "pop     r25                             \n\t");

  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}


ISR(TIMER2_COMPB_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t");

  PORTB = portb_val;

  register bool is_beat_1 asm ("r24") = F.is_beat_1;
  if(is_beat_1) beat_pin.high(); // this compiles to a `sbrc` which doesn't affect the SREG!

  register bool is_beat_2 asm ("r24") = F.is_beat_2;
  if(is_beat_2) tempo_pin.high(); // this compiles to a `sbrc` which doesn't affect the SREG!
  
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}
