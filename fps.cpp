/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "fps.h"

uint32_t start_time = 0; // time each loop started.
uint32_t silent_since = 0; // time we've been silent since.

uint32_t last_delay = 0;

int8_t volatile fps_interrupt_count = FPS_INTERRUPT_RESET_VAL;

void setup_fps() {
  // // TIMER1 is used for the sampler. We are going to piggy back on Timer 1 to get an interrupt to use for keeping the frame rate steady.
  // OCR1B = 0;
  // // enable timer compare interrupt
  // TIMSK1 |= (1 << OCIE1B);
}

/* N.B. we apply fps_count() in ledpwm.cpp in order to efficiently share an interrupt */

// ISR(TIMER1_COMPB_vect)
// {
//   fps_count(SAMP_FREQ / FPS); // defined in header.
// }

