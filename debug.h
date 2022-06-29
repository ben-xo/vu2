/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _DEBUG_H
#define _DEBUG_H

#include <Arduino.h>
#include <DigitalIO.h>
#include "config.h"


#ifdef DEBUG_SAMPLE_RATE_PIN
  extern DigitalPin<DEBUG_SAMPLE_RATE_PIN> debug_sample_rate_pin;
  #define DEBUG_SAMPLE_RATE_HIGH() (debug_sample_rate_pin.high())
  #define DEBUG_SAMPLE_RATE_LOW()  (debug_sample_rate_pin.low())
#else
  #define DEBUG_SAMPLE_RATE_HIGH() ()
  #define DEBUG_SAMPLE_RATE_LOW()  ()
#endif

#ifdef DEBUG_FRAME_RATE_PIN
  extern DigitalPin<DEBUG_FRAME_RATE_PIN> debug_frame_rate_pin;
  #define DEBUG_FRAME_RATE_HIGH() (debug_frame_rate_pin.high())
  #define DEBUG_FRAME_RATE_LOW()  (debug_frame_rate_pin.low())
#else
  #define DEBUG_FRAME_RATE_HIGH() ()
  #define DEBUG_FRAME_RATE_LOW()  ()
#endif

#ifdef DEBUG_AUDIO_PROCESSING_RATE_PIN
  extern DigitalPin<DEBUG_AUDIO_PROCESSING_RATE_PIN> debug_audio_processing_rate_pin;
  #define DEBUG_AUDIO_PROCESSING_RATE_HIGH() (debug_audio_processing_rate_pin.high())
  #define DEBUG_AUDIO_PROCESSING_RATE_LOW()  (debug_audio_processing_rate_pin.low())
#else
  #define DEBUG_AUDIO_PROCESSING_RATE_HIGH() ()
  #define DEBUG_AUDIO_PROCESSING_RATE_LOW()  ()
#endif

void setup_debug();

#endif /* _DEBUG_H */
