/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// This file is included by all LED strip audio effects.

#ifndef _EFFECTS_H
#define _EFFECTS_H

#define VU_PER_PIXEL (256 / STRIP_LENGTH)

#include <Arduino.h>
#include "config.h"
#include <FastLED.h>
#include "color_wheels.h"

extern CRGB leds[STRIP_LENGTH];
extern byte samples[SAMP_BUFF_LEN];
extern volatile uint8_t current_sample;

// effects.cpp
void fade_pixel(uint8_t pixel);
void fade_pixel_slow(uint8_t pixel);
void fade_pixel_fast(uint8_t pixel);

// startup.cpp
void do_banner();

// attract.cpp
void setup_attract();
void render_attract();

#endif /* _EFFECTS_H */
