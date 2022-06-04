/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _DEBUG_H
#define _DEBUG_H

#include <Arduino.h>
#include "config.h"

#include "ledpwm.h"
#include "sampler.h"
#include "beatdetect.h"
#include "buttons.h"
#include "fps.h"

#include "sevenseg.h"

#include "debugrender.h"

#ifdef DEBUG_SAMPLE_RATE
  #define DEBUG_SAMPLE_RATE_HIGH() (DEBUG_SAMPLE_RATE_PORT |= (1 << DEBUG_SAMPLE_RATE_PIN))
  #define DEBUG_SAMPLE_RATE_LOW()  (DEBUG_SAMPLE_RATE_PORT &= ~(1 << DEBUG_SAMPLE_RATE_PIN))
#else
  #define DEBUG_SAMPLE_RATE_HIGH() ()
  #define DEBUG_SAMPLE_RATE_LOW()  ()
#endif

#ifdef DEBUG_FRAME_RATE
  #define DEBUG_FRAME_RATE_HIGH() (DEBUG_FRAME_RATE_PORT |= (1 << DEBUG_FRAME_RATE_PIN))
  #define DEBUG_FRAME_RATE_LOW()  (DEBUG_FRAME_RATE_PORT &= ~(1 << DEBUG_FRAME_RATE_PIN))
#else
  #define DEBUG_FRAME_RATE_HIGH() ()
  #define DEBUG_FRAME_RATE_LOW()  ()
#endif

extern uint32_t start_time;
extern uint32_t silent_since;

void setup_debug();
void debug_loop();

#endif /* _DEBUG_H */
