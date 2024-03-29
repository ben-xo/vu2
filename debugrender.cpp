/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "debugrender.h"

// These are example renderers, really just for visualising what's being sampled.

void debug_render_combo(bool is_beat, bool is_beat_2, uint8_t sample_ptr) {
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
      // the +1 and +2 just make it a bit more colourful on the strip…
      uint8_t r = sampler.samples[(sample_ptr + j) % SAMP_BUFF_LEN];
      uint8_t g = sampler.samples[(sample_ptr + j*3) % SAMP_BUFF_LEN];
      uint8_t b = sampler.samples[(sample_ptr + j*5) % SAMP_BUFF_LEN];
      leds[j].setRGB(
        r == 1 ? 0 : is_beat_2 ? r : 0, 
        g == 1 ? 0 : is_beat ? g : 0, 
        b == 1 ? 0 : is_beat ? 0 : b
       );
    }
}

void debug_render_is_beat(bool is_beat_2, bool is_beat_1) {
    FastLED.clear();
    leds[1].setRGB( 
      is_beat_1 ? 64 : 0, 
      is_beat_2 ? 64 : 0,
      is_beat_2 ? 64 : 0
     );
}

void debug_render_samples(uint8_t sample_ptr, bool colourful) {
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
      // the +1 and +2 just make it a bit more colourful on the strip…
      int8_t r = (int8_t)(sampler.samples[(sample_ptr + j) % SAMP_BUFF_LEN] - 127);
      int8_t g = (int8_t)(sampler.samples[(sample_ptr + (colourful ? j*3 : j)) % SAMP_BUFF_LEN] - 127);
      int8_t b = (int8_t)(sampler.samples[(sample_ptr + (colourful ? j*5 : j)) % SAMP_BUFF_LEN] - 127);
//      Serial.print(r); Serial.print("\n");
      uint8_t rr = (r < 0 ? 127+r : r);
      uint8_t gg = (g < 0 ? 127+g : g);
      uint8_t bb = (b < 0 ? 127+b : b);
      leds[j].setRGB(
        rr < 4 ? 0 : rr, 
        gg < 4 ? 0 : gg, 
        bb < 4 ? 0 : bb
       );
    }
}

void debug_render_vu(uint8_t vu_width) {
    uint16_t j = 0;
    while (j < vu_width/2) {
      leds[j].setRGB(0, 16, 0);
      j++;
    }
    j = vu_width/2;
    while(j < STRIP_LENGTH) {
//      uint32_t oldColor = strip.getPixelColor(j);
//      uint8_t old_r = (oldColor & 0x00ff0000) >> 16;
//      uint8_t old_g = (oldColor & 0x0000ff00) >> 8;
//      uint8_t old_b = (oldColor & 0x000000ff);
//      strip.setPixelColor(j, old_r * 0.95, old_g * 0.95, old_b * 0.95);
        leds[j] = scale8(leds[j], 0.95);
//      strip.setPixelColor(j,0,0,0);
      j++;
    }
}
