/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _FPS_H
#define _FPS_H

#include <Arduino.h>
#include "config.h"

extern uint32_t start_time;   // time each loop started.
extern uint32_t silent_since; // time we've been silent since.
extern volatile bool slow;             // track render time

void setup_fps();
void reach_target_fps();

extern int8_t fps_interrupt_count;

#define INTERRUPT_RESET_VAL (PWM_LED_FRQ / FPS)

// void __inline__ fps_count()
// {
 
//   // // using an intermediate variable makes the compiled output much more efficient.
//   // int8_t new_interrupt_count = fps_interrupt_count - 1;
//   // if(!new_interrupt_count) {
//   //   GPIOR0 |= (1<<1);
//   //   new_interrupt_count = interrupt_reset_val;
//   // }
//   // fps_interrupt_count = new_interrupt_count;

//   asm volatile( 
//     "lds r24, fps_interrupt_count            \n\t" 
//     "subi  r24, 0x01            \n\t" 
//     "brne  .+4            \n\t" 
//     "sbi %0, 1            \n\t" 
//     "ldi r24, %1        \n\t" 
//     "sts fps_interrupt_count, r24        \n\t" 
//     : 
//     : 
//     "I" (_SFR_IO_ADDR(GPIOR0)),
//     "M" (INTERRUPT_RESET_VAL)

//   );

//   // // using an intermediate variable makes the compiled output much more efficient.
//   // int8_t new_interrupt_count = fps_interrupt_count - 1;
//   //   156c: 80 91 00 01   lds r24, 0x0100 ; 0x800100 <fps_interrupt_count>
//   //   1570: 81 50         subi  r24, 0x01 ; 1
//   // if(!new_interrupt_count) {
//   //   1572: 11 f4         brne  .+4       ; 0x1578 <__vector_7+0x1a>
//   //   GPIOR0 |= (1<<1);
//   //   1574: f1 9a         sbi 0x1e, 1 ; 30
//   //   new_interrupt_count = interrupt_reset_val;
//   //   1576: 82 e4         ldi r24, 0x42 ; 66
//   // }
//   // fps_interrupt_count = new_interrupt_count;
//   //   1578: 80 93 00 01   sts 0x0100, r24 ; 0x800100 <fps_interrupt_count>  
// }

#endif /* _FPS_H */
