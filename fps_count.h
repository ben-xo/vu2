/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


/*
 * The expectation is that you only include this in ledpwm.cpp.
 * It's not really related, which is why it has its own file, but it should still be inlined
 */

#ifndef _FPS_COUNT_H
#define _FPS_COUNT_H

#include "gpio0.h"
#include "fps_constants.h"

// this is defined in fps.cpp
extern int8_t volatile fps_interrupt_count;

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
static void inline __attribute__((always_inline)) fps_count()
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
    "brne  .+4            \n\t" 

    //   GPIOR0 |= (END_OF_FRAME_FLAG);
    "sbi %[gpior0_addr], %[eoff]            \n\t" 

    //   new_interrupt_count = interrupt_reset_val;
    "ldi r24, %[irv]        \n\t" 
    // }

    // fps_interrupt_count = new_interrupt_count;
    "sts %[fic], r24        \n\t" 
    : 
    : 
    [gpior0_addr] "I" (_SFR_IO_ADDR(GPIOR0)),
    [irv] "M" (FPS_INTERRUPT_RESET_VAL),
    [eoff] "I" (END_OF_FRAME_FLAG_BIT),
    [fic] "i" (fic)

  );
}

#endif /* _FPS_COUNT_H */
