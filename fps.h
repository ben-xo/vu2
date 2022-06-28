/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _FPS_H
#define _FPS_H

#include <Arduino.h>
#include "config.h"

extern uint32_t start_time;   // time each loop started.
extern uint32_t silent_since; // time we've been silent since.

void setup_fps();

extern volatile int8_t fps_interrupt_count;

#define INTERRUPT_RESET_VAL (PWM_LED_FRQ / FPS)

/*
 * Busy-wait until the end of the frame, then clear the end-of-frame flag.
 */
__attribute__((always_inline)) static void reach_target_fps()
{
  uint32_t end_time = micros();
  while(!(GPIOR0 & (1<<1))) {
    asm("");
  }
  GPIOR0 &= ~(1<<1);
  start_time = end_time;
}


#endif /* _FPS_H */
