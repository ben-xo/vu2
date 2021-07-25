/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "effects.h"
#include "renderheap.h"


#define POS_PER_PIXEL (32768 / STRIP_LENGTH) // ratio of attract mode pixel-positions to actual LEDs

static void make_new_dot(uint8_t dot) {
    r.am.dot_colors[dot] = Wheel(random8());
    r.am.dot_pos[dot] = random16(10000); // 32768 * 0.2 (so pixels dont start at the very end of the strip)
    r.am.dot_speeds[dot] = random16(POS_PER_PIXEL/17,POS_PER_PIXEL/5);
    r.am.dot_age[dot] = 0;
}

static void calculate_overtakes(uint8_t dot) {
  for(uint8_t i = 0; i < ATTRACT_MODE_DOTS; i++) {
    if((r.am.dot_pos[dot] < r.am.dot_pos[i]) && (r.am.dot_pos[dot] + r.am.dot_speeds[dot]) >= r.am.dot_pos[i]) {
      // i will overtake j on next render. Let's make them collide!
      uint8_t new_r = blend8(r.am.dot_colors[i].r, r.am.dot_colors[dot].r, 127);
      uint8_t new_g = blend8(r.am.dot_colors[i].g, r.am.dot_colors[dot].g, 127);
      uint8_t new_b = blend8(r.am.dot_colors[i].b, r.am.dot_colors[dot].b, 127);
      r.am.dot_colors[i] = CRGB(new_r,new_g,new_b);
      r.am.dot_colors[dot] = r.am.dot_colors[i];
    }
  }
}

void setup_attract() {
  for(uint8_t i = 0; i < ATTRACT_MODE_DOTS; i++) {
    make_new_dot(i);
  }
}

// TODO: optimise for FastLED
void render_attract() {

  uint16_t leds_offsets[ATTRACT_MODE_DOTS] = {0};

  // fade out (trails, but also between non-attract modes and this mode)
  for(uint8_t i = 0; i < STRIP_LENGTH; i++) {
    fade_pixel(i);
  }

  // this loop scales the dots from their "native res" (0-32768) antialiased to the LED strip (0-60)
  // and keeps track of which LED pixels have actually been rendered to (which is 2x number of dots)
  for(uint8_t dot=0; dot < ATTRACT_MODE_DOTS; dot++) {
    if(r.am.dot_pos[dot] >= 32768) {
      make_new_dot(dot);
    } 
    // draw adjusted to strip (trying to do anti-aliasing)
    // ratio of pixel in left or right pixels
    uint16_t led = r.am.dot_pos[dot] / POS_PER_PIXEL;
    leds_offsets[dot] = (r.am.dot_pos[dot] % (POS_PER_PIXEL));
    calculate_overtakes(dot);
    r.am.dot_pos[dot] += r.am.dot_speeds[dot];
    if(r.am.dot_age[dot] < 255) r.am.dot_age[dot]++;

    if(led+1 < STRIP_LENGTH) {
      CRGB old_color = leds[led];
      uint8_t old_r = old_color.r;
      uint8_t old_g = old_color.g;
      uint8_t old_b = old_color.b;
      uint8_t dot_r = r.am.dot_colors[dot].r;
      uint8_t dot_g = r.am.dot_colors[dot].g;
      uint8_t dot_b = r.am.dot_colors[dot].b;
      
      uint16_t new_r = old_r + dot_r * (r.am.dot_age[dot]/255.0) * (POS_PER_PIXEL - leds_offsets[dot])/POS_PER_PIXEL;
      uint16_t new_g = old_g + dot_g * (r.am.dot_age[dot]/255.0) * (POS_PER_PIXEL - leds_offsets[dot])/POS_PER_PIXEL;
      uint16_t new_b = old_b + dot_b * (r.am.dot_age[dot]/255.0) * (POS_PER_PIXEL - leds_offsets[dot])/POS_PER_PIXEL;
      leds[led+1] = CRGB( 
        min(new_r, 255),
        min(new_g, 255), 
        min(new_b, 255)
      );
//      old_color = strip.getPixelColor(led+1);
//      old_r = (uint8_t)(old_color >> 16);
//      old_g = (uint8_t)(old_color >> 8 );
//      old_b = (uint8_t)(old_color      );
//      r = old_r + dot_r * (dot_age[dot]/255.0) * (leds_offsets[dot])/POS_PER_PIXEL;
//      g = old_g + dot_g * (dot_age[dot]/255.0) * (leds_offsets[dot])/POS_PER_PIXEL;
//      b = old_b + dot_b * (dot_age[dot]/255.0) * (leds_offsets[dot])/POS_PER_PIXEL;
//      strip.setPixelColor(led+1, 
//        min(r, 255),
//        min(g, 255), 
//        min(b, 255)
//      );
    }
  }
}
