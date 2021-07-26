/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include <FastLED.h>
#include "config.h"
#include "sampler.h"
extern CRGB leds[STRIP_LENGTH*2 + (STRIP_LENGTH/3)];
void debug_render_combo(bool is_beat, bool is_beat_2, uint8_t sample_ptr);
void debug_render_is_beat(bool is_beat_1, bool is_beat_2);
void debug_render_samples(uint8_t sample_ptr, bool colourful);
void debug_render_vu(uint8_t vu_width);

#define MAX_DEBUG_MODE 2
