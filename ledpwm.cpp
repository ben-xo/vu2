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
#include "fps.h"
#include "gpio0.h"

#include "fps_constants.h"

int8_t volatile fps_interrupt_count = INTERRUPT_RESET_VAL;

/* definition to expand macro then apply to pragma message */
/* from https://stackoverflow.com/questions/1562074/how-do-i-show-the-value-of-a-define-at-compile-time */
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

// compile time debug to see the PWM vals
#pragma message(VAR_NAME_VALUE(PWM_OVERFLOW_VALUE))
#pragma message(VAR_NAME_VALUE(PWM_DUTY_VALUE))

// we're attaching the FPS calculation to the ledpwm interrupt to lower the number of interrupts.
#include "fps_count.h"

// we're attaching the sampler to the ledpwm interrupt to lower the number of interrupts.
#include "sampler.h"


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

  portb_mask = 0b01111111;

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

  // well, we needed to push SREG, so might as well do this now, as ROR affects SREG
  // 
  uint8_t temp = portb_mask;
  portb_mask = (temp >> 1) | (temp << 7); // basically, ror

  fps_count();

  asm volatile( "out     %0, r25                         \n\t" :: "I" (_SFR_IO_ADDR(SREG)));
  asm volatile( "pop     r25                             \n\t");

  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}


ISR(TIMER2_COMPB_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t");

  // Based on LSB of portb_mask, swap the nibbles of portb val before displaying.
  // The idea is that portb_val is actually a double buffer, and portb_mask is effectively
  // a blend percentage. Once every sample interrupt, it is rotated by 1 bit.
  // So a mask = 0x00 will always show the one half of portb_val, and mask = 0xFF will show the other half,
  // with mask = 0x55 showing a 50/50 mix. Thus, you can achieve fades and pulses on the seven seg
  // by periodically updating the val and the mask.
  asm volatile(
    "in	r24, %[portb_mask_io_reg] \n\t"
    "cbi	%[flags_io_reg], 2 \n\t"
    "sbrc	r24, 0 \n\t"
    "sbi	%[flags_io_reg], 2 \n\t"
    "in	r24, %[portb_val_io_reg] \n\t"
    "sbic	%[flags_io_reg], 2 \n\t"
    "swap r24 \n\t"
    "out %[portb_io_reg], r24 \n\t"
    :: 
    [portb_mask_io_reg] "I" (_SFR_IO_ADDR(portb_mask)),
    [portb_val_io_reg] "I" (_SFR_IO_ADDR(portb_val)),
    [flags_io_reg] "I" (_SFR_IO_ADDR(GPIOR0)),
    [portb_io_reg] "I" (_SFR_IO_ADDR(PORTB))
  );


  register bool is_beat_1 asm ("r24") = F.is_beat_1;
  if(is_beat_1) beat_pin.high(); // this compiles to a `sbrc` which doesn't affect the SREG!

  register bool is_beat_2 asm ("r24") = F.is_beat_2;
  if(is_beat_2) tempo_pin.high(); // this compiles to a `sbrc` which doesn't affect the SREG!

  if(!(GPIOR0 & (1<<0))) {
    GPIOR0 |= (1<<0);
    asm volatile( "pop     r24                             \n\t");
    asm volatile( "reti                                    \n\t");
  }

  GPIOR0 &= ~(1<<0);

  asm volatile(
    "push  r30 \t\n"
    "in  r30, __SREG__ \t\n"
    // "push  r24 \t\n" // in ISR_NAKED prologue
    "push  r30 \t\n"
    "push  r31 \t\n"
  );

  sample();
  
  asm volatile(
    "pop r31 \t\n"
    "pop r30 \t\n"
    // "pop r24 \t\n" // in ISR_NAKED epilogue
    "out __SREG__, r30\t\n"
    "pop r30 \t\n"
  );
  asm volatile( "pop     r24                             \n\t");
  asm volatile( "reti                                    \n\t");
}
