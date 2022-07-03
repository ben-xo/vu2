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

#include "pwm_constants.h"

// we're attaching the FPS calculation to the ledpwm interrupt to lower the number of interrupts.
#include "fps_count.h"

// we're attaching the sampler to the ledpwm interrupt to lower the number of interrupts.
#include "sampler.h"

void setup_ledpwm() {
  cli();
  // Clear registers
  TCCR2B = 0; // ensure the timer starts disabled. We call enable_ledpwm() at the end
  TCCR2A = 0;

  // The correct formula here is (F_CPU / ((OCR2A+1)*PRESCALER)) - google why
  OCR2A = PWM_OVERFLOW_VALUE - 1; // 24 for 10kHz with prescaler 64, 199 for 10kHz with prescaler 8
  OCR2B = PWM_DUTY_VALUE;         // duty cycle, usually ~10% of OCR2A
  
  // CTC mode
  TCCR2A = (1 << WGM21);
  
  // Output Compare Match A & B Interrupt Enable
  // TIMER2_COMPA_vect clears the LEDs, TIMER2_COMPB_vect lights them.
  TIMSK2 |= (1 << OCIE2A) | (1 << OCIE2B);

  clear_status_leds_within_interrupt();

  // this clears the timer and sets the right pre-scaler, starting the timer.
  enable_ledpwm();
  sei();
}

/* 
 * disable the timer entirely, then make sure the lights are off.
 *
 * *NOTE* this also disables the sampler and end-of-frame flag, so calling this
 * is not recommended unless you're doing your own loop outside of the main loop.
 */
void disable_ledpwm() {
  cli();
  TCCR2B = 0;
  clear_status_leds_within_interrupt();
  sei();
}

/*
 * enable the pwm interrupt timer.
 *
 * Starts the timer at an offset so that PWM interrupts don't coincide with other interrupts.
 * Without this, sometimes we get unstable PWM when interrupts pile up.
 */
void enable_ledpwm() {
  TCNT2 = PWM_STARTING_OFFSET;
  
#if (PWM_PRESCALER == 8)
  TCCR2B = (1 << CS21); // enable the timer (with pre-scaler 8)
#elif (PWM_PRESCALER == 64)
  TCCR2B = (1 << CS22); // enable the timer (with pre-scaler 64)
#else
# error PWM_PRESCALER must be 8 or 64
#endif
}

/*
 * This interrupt fires to turn the lights out, both on PORTB and the two beat pins.
 * It also rotates the LED brigtness mask and does the FPS count.
 *
 * Lights out: 4 cycles
 * Mask rotate: 5 cycles
 * FPS count:  7 or 8 cycles
 * Interrupt overhead: 14 cycles
 * Back Buffer rotate: 2 or 12 cycles
 * Total cycles: 32 or 33 or 44 or 45 cycles
 */
ISR(TIMER2_COMPA_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t"); // 2cy

  // "PORTB = 0" would set PORTB from r1, but we can't guarantee that's 0.
  // ldi rN, 0 doesn't affect SREG, but we can't ldi into r1 (has to be r15+)
  // so, do it manually

  asm volatile( "ldi     r24, 0                          \n\t"); // 1cy
  asm volatile( "out     %0, r24     ; PORTB             \n\t" :: "I" (_SFR_IO_ADDR(PORTB))); // 1cy

  beat_pin.low();  // 1cy
  tempo_pin.low(); // 1cy

  // unfortunately we need to back up SREG for the mask rotate and fps_count
  asm volatile( "push    r25                             \n\t"); // 2cy
  asm volatile( "in      r25, __SREG__                   \n\t"); // 1cy

  // if(GPIOR0 & (LEDPWM_ROTATE_BACK_BUFFER_FLAG)) {
  //   asm volatile( "push    r25                             \n\t"); // 2cy
  //   temp = portb_val;
  //   if ((temp << 1) < 0) {
  //     temp |= 0b00010000;
  //   }
  //   portb_val = temp;
  //   asm volatile( "pop     r25                             \n\t"); // 2cy
  // }

  // if(GPIOR0 & (LEDPWM_ROTATE_BACK_BUFFER_FLAG)) {
  //   temp = portb_val;
  //   temp &= 0x0F;
  //   asm volatile(
  //     "cp %[portb_val_io_reg], %[portb_val_io_reg]       \n\t"
  //     "addi r24, "
  //     "ori  r24, 0b01000000                              \n\t"
  //     "sbis %[portb_val_io_reg], 6                       \n\t"
  //     "ori  r24, 0b00100000                              \n\t"
  //     "sbis %[portb_val_io_reg], 5                       \n\t"
  //     "ori  r24, 0b00010000                              \n\t"
  //     "sbis %[portb_val_io_reg], 4                       \n\t"
  //     "ori  r24, 0b10000000                              \n\t"
  //     ::
  //     [portb_val_io_reg] "I" (_SFR_IO_ADDR(portb_val))
  //   ); // 2cy
  //   portb_val = temp;
  // }

  fps_count();                                                   // 7 or 8cy

  // Rotate the portb_mask (this is used for brightness control on LEDs in the other interrupt.)
  // As we alreaded needed to push SREG we might as well do this now, as `ror` affects SREG

  uint8_t register temp asm("r24") = portb_mask;  // 1 cy
  temp = (temp >> 1) | (temp << 7); // basically, ror -    // 3 cy
  portb_mask = temp; // 1 cy

  asm volatile( "out     __SREG__, r25                   \n\t"); // 1cy
  asm volatile( "pop     r25                             \n\t"); // 2cy
  asm volatile( "pop     r24                             \n\t"); // 2cy
  asm volatile( "reti                                    \n\t"); // 4cy
}

/*
 * This interrupt fires to turn the LED lights on. The LSB of the mask determines 
 * whether we `swap` the value of PORTB before showing it (i.e. double buffered upper or lower half)
 *
 * NOTE: based on the following calculation:
 *
 * 16,000,000 cycles/sec
 *  10,000 OCR2A interrupts / sec with overflow val 199 (prescaler 8, so that's every 1600 clocks)
 *  10,000 OCR2B interrupts / sec overflow val 190, so 10*8 (80) cycles before OCR2A for duty cycle 10%
 *
 * _The max length of this interrupt is currently 80 cycles, before it risks making OCR2A fire late._
 *
 * 67 cycles for the sampler path (sampler takes ~31)
 * 21 cycles for the non-sampler path
 */
ISR(TIMER2_COMPB_vect, ISR_NAKED) {
  asm volatile( "push    r24                             \n\t"); // 2cy

  register bool is_beat_1 asm ("r24") = F.is_beat_1; // 2cy
  if(is_beat_1) beat_pin.high(); // this compiles to a `sbrc` which doesn't affect the SREG!

  register bool is_beat_2 asm ("r24") = F.is_beat_2; // 2cy
  if(is_beat_2) tempo_pin.high(); // this compiles to a `sbrc` which doesn't affect the SREG!

  /* Based on LSB of portb_mask, swap the nibbles of portb val before displaying.
   * The idea is that portb_val is actually a double buffer, and portb_mask is effectively
   * a blend percentage. Once every sample interrupt, it is rotated by 1 bit.
   * So a mask = 0x00 will always show the one half of portb_val, and mask = 0xFF will show the other half,
   * with mask = 0x55 showing a 50/50 mix. Thus, you can achieve fades and pulses on the seven seg
   * by periodically updating the val and the mask.
   *
   * Note that because just saving and restoring SREG takes 6 cycles, we're avoiding anything that modifies
   * SREG altogether in order to keep this to 7 cycles total.
   *
   * total: 7 cycles
   */
  asm volatile(
    // the mask is rotated in the other interrupt; we test the LSB of the mask to decide if we are swapping to the back buffer

    "in r24, %[portb_mask_io_reg] \n\t"
    "sbrc r24, 0 \n\t"
    "rjmp .+4 \n\t"

    "in r24, %[portb_val_io_reg] \n\t"
    "rjmp .+4 \n\t"

    "in r24, %[portb_val_io_reg] \n\t"
    "swap r24 \n\t"
    "out %[portb_io_reg], r24 \n\t"

    :: 
    [portb_mask_io_reg] "I" (_SFR_IO_ADDR(portb_mask)),
    [portb_val_io_reg] "I" (_SFR_IO_ADDR(portb_val)),
    [portb_io_reg] "I" (_SFR_IO_ADDR(PORTB))
  ); // 7cy


  // return early every other frame (i.e. sampler runs at half PWM freq)
  if(!(GPIOR0 & (EVERY_OTHER_FRAME_FLAG))) {
    // test itself takes 1 cy

    GPIOR0 |= (EVERY_OTHER_FRAME_FLAG); // 1 cy

    asm volatile( "pop     r24                             \n\t"); // 2cy
    asm volatile( "reti                                    \n\t"); // 4cy
  } // 8 cy if returning, 2 cy otherwise

  GPIOR0 &= ~(EVERY_OTHER_FRAME_FLAG); // 1 cy

  asm volatile(
    "push  r30 \t\n"
    "in  r30, __SREG__ \t\n"
    // "push  r24 \t\n" // in ISR_NAKED prologue
    "push  r30 \t\n"
    "push  r31 \t\n"
  ); // 7 cy

  sample(); // ~ 31 cy
  
  asm volatile(
    "pop r31 \t\n"
    "pop r30 \t\n"
    // "pop r24 \t\n" // in ISR_NAKED epilogue
    "out __SREG__, r30\t\n"
    "pop r30 \t\n"
  ); // 7 cy
  asm volatile( "pop     r24                             \n\t"); // 2cy
  asm volatile( "reti                                    \n\t"); // 4cy
}
