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

void __inline__ fps_count(int8_t interrupt_reset_val)
{
 
  // using an intermediate variable makes the compiled output much more efficient.
  int8_t new_interrupt_count = fps_interrupt_count - 1;
  if(!new_interrupt_count) {
    GPIOR0 |= (1<<1);
    new_interrupt_count = interrupt_reset_val;
  }
  fps_interrupt_count = new_interrupt_count;
}

#endif /* _FPS_H */
