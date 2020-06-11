/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// helpers used by many effects.

#include "effects.h"

void fade_pixel(uint8_t pixel) {
  uint8_t r = leds[pixel].r;
  uint8_t g = leds[pixel].g;
  uint8_t b = leds[pixel].b;
  // this is equivalent to r*0.875
  leds[pixel] = CRGB(qsub8(r, (r>>3)+1), qsub8(g, (g>>3)+1), qsub8(b, (b>>3)+1));
}

void fade_pixel_slow(uint8_t pixel) {
  uint8_t r = leds[pixel].r;
  uint8_t g = leds[pixel].g;
  uint8_t b = leds[pixel].b;
  // this is equivalent to r*0.9375
  leds[pixel] = CRGB(qsub8(r, (r>>4)+1), qsub8(g, (g>>4)+1), qsub8(b, (b>>4)+1));
}

void fade_pixel_fast(uint8_t pixel) {
  uint8_t r = leds[pixel].r;
  uint8_t g = leds[pixel].g;
  uint8_t b = leds[pixel].b;
  leds[pixel] = CRGB(qsub8(r, (r>>2)+1), qsub8(g, (g>>2)+1), qsub8(b, (b>>2)+1));
  // this is equivalent to r*0.75
}
