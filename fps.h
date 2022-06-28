/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _FPS_H
#define _FPS_H

#include <Arduino.h>
#include "config.h"

// defines END_OF_FRAME_FLAG for GPIO0
#include "gpio0.h"

extern uint32_t start_time;   // time each loop started.
extern uint32_t silent_since; // time we've been silent since.


extern volatile uint8_t fps_interrupt_count;

#define INTERRUPT_RESET_VAL (PWM_LED_FRQ / FPS)

__attribute__((always_inline)) static void setup_fps() 
{
  start_time = micros(); // time each loop started.
  silent_since = start_time; // time we've been silent since.
  fps_interrupt_count = INTERRUPT_RESET_VAL;
}

/*
 * Every time fps_count() is called, it decrements a counter of "interrupts per frame".
 *
 * We hitch a ride on the LED PWM interrupt (see ledpwm.cpp), as the timing there
 * is essentially jitter free.
 *
 * For example, if our desired frame rate is 150FPS, and our interrupt triggers 10k times
 * per second (i.e. a typical LED PWM value), then we count down from 200 to 0 and set a
 * flag in GPIOR0 when it underflows. (See PWM_OVERFLOW_VALUE in ledpwm.cpp)
 *
 * GPIOR0 is chosen because you can use `sbi` to set the bit very quickly.
 *
 * This whole function only takes 7 or 8 clock cycles.
 */
__attribute__((always_inline)) static void __inline__ fps_count()
{
  // clobbers r24 and SREG so they must be saved before use.
  // stores overflow in GPIOR0:1. So, that must be reset when read at point of use.

  // // using an intermediate variable makes the compiled output much more efficient.
  // int8_t new_interrupt_count = fps_interrupt_count - 1;
  // if(!new_interrupt_count) {
  //   GPIOR0 |= (END_OF_FRAME_FLAG);
  //   new_interrupt_count = interrupt_reset_val;
  // }
  // fps_interrupt_count = new_interrupt_count;

  volatile uint8_t* fic = &fps_interrupt_count; 
  asm volatile( 
    // int8_t new_interrupt_count = fps_interrupt_count - 1;
    "lds r24, %[fic]            \n\t" 
    "subi  r24, 0x01            \n\t" 

    // if(!new_interrupt_count) {
    "brne  .+4                  \n\t" 

    //   GPIOR0 |= (END_OF_FRAME_FLAG);
    "sbi %[gpio0_addr], %[_END_OF_FRAME_FLAG]                  \n\t" 

    //   new_interrupt_count = interrupt_reset_val;
    "ldi r24, %[_INTERRUPT_RESET_VAL]                \n\t" 
    // }

    // fps_interrupt_count = new_interrupt_count;
    "sts %[fic], r24            \n\t" 
    : 
    : 
    [gpio0_addr] "I" (_SFR_IO_ADDR(GPIOR0)),
    [_INTERRUPT_RESET_VAL] "M" (INTERRUPT_RESET_VAL),
    [_END_OF_FRAME_FLAG] "M" (END_OF_FRAME_FLAG),
    [fic] "i" (fic)

  );
}

/*
 * Busy-wait until the end of the frame, then clear the end-of-frame flag.
 */
__attribute__((always_inline)) static void reach_target_fps()
{
  uint32_t end_time = micros();
  while(!(GPIOR0 & (END_OF_FRAME_FLAG))) {
    asm("");
  }
  GPIOR0 &= ~(END_OF_FRAME_FLAG);
  start_time = end_time;
}

#endif /* _FPS_H */

