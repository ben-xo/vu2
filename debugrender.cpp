#include "debugrender.h"

// These are example renderers, really just for visualising what's being sampled.

void debug_render_combo(UltraFastNeoPixel the_strip, bool is_beat, uint8_t sample_ptr) {
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
      // the +1 and +2 just make it a bit more colourful on the strip…
      uint8_t r = samples[(sample_ptr + j) % SAMP_BUFF_LEN];
      uint8_t g = samples[(sample_ptr + j*3) % SAMP_BUFF_LEN];
      uint8_t b = samples[(sample_ptr + j*5) % SAMP_BUFF_LEN];
      the_strip.setPixelColor(j, 
        r == 1 ? 0 : is_beat ? r : 0, 
        g == 1 ? 0 : is_beat ? g : 0, 
        b == 1 ? 0 : is_beat ? 0 : b
       );
    }
}

void debug_render_is_beat(UltraFastNeoPixel the_strip, bool is_beat) {
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
      the_strip.setPixelColor(j, 
        is_beat ? 64 : 0, 
        is_beat ? 64 : 0, 
        is_beat ? 64 : 0
       );
    }
}

void debug_render_samples(UltraFastNeoPixel the_strip, uint8_t sample_ptr, bool colourful) {
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
      // the +1 and +2 just make it a bit more colourful on the strip…
      uint8_t r = samples[(sample_ptr + j) % SAMP_BUFF_LEN];
      uint8_t g = samples[(sample_ptr + (colourful ? j*3 : j)) % SAMP_BUFF_LEN];
      uint8_t b = samples[(sample_ptr + (colourful ? j*5 : j)) % SAMP_BUFF_LEN];
      the_strip.setPixelColor(j, 
        r == 1 ? 0 : r, 
        g == 1 ? 0 : g, 
        b == 1 ? 0 : b
       );
    }
}

void debug_render_vu(UltraFastNeoPixel the_strip, uint8_t vu_width) {
    for (uint8_t j = 0; j < vu_width/4; j++) {
      the_strip.setPixelColor(j, 32, 32, 32);
    }
    for (uint8_t j = vu_width/4; j < STRIP_LENGTH; j++) {
      uint32_t oldColor = the_strip.getPixelColor(j);
      the_strip.setPixelColor(j, 0,0,0);
    }
}
