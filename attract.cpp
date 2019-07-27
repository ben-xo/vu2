#include "effects.h"

#define POS_PER_PIXEL (32768 / STRIP_LENGTH) // ratio of attract mode pixel-positions to actual LEDs

static CRGB dot_colors[ATTRACT_MODE_DOTS];
static uint32_t dot_pos[ATTRACT_MODE_DOTS];
static uint16_t dot_speeds[ATTRACT_MODE_DOTS];
static uint8_t dot_age[ATTRACT_MODE_DOTS];

static void make_new_dot(uint8_t dot) {
    dot_colors[dot] = Wheel(random8());
    dot_pos[dot] = random16(10000); // 32768 * 0.2 (so pixels dont start at the very end of the strip)
    dot_speeds[dot] = random16(POS_PER_PIXEL/17,POS_PER_PIXEL/5);
    dot_age[dot] = 0;
}

static void calculate_overtakes(uint8_t dot) {
  for(uint8_t i = 0; i < ATTRACT_MODE_DOTS; i++) {
    if((dot_pos[dot] < dot_pos[i]) && (dot_pos[dot] + dot_speeds[dot]) >= dot_pos[i]) {
      // i will overtake j on next render. Let's make them collide!
      uint8_t r = ((uint8_t)(dot_colors[i].r) + (uint8_t)(dot_colors[dot].r)) >> 1;
      uint8_t g = ((uint8_t)(dot_colors[i].g) + (uint8_t)(dot_colors[dot].g)) >> 1;
      uint8_t b = ((uint8_t)(dot_colors[i].b) + (uint8_t)(dot_colors[dot].b)) >> 1;
      dot_colors[i] = CRGB(r,g,b);
      dot_colors[dot] = dot_colors[i];
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
    if(dot_pos[dot] >= 32768) {
      make_new_dot(dot);
    } 
    // draw adjusted to strip (trying to do anti-aliasing)
    // ratio of pixel in left or right pixels
    uint16_t led = dot_pos[dot] / POS_PER_PIXEL;
    leds_offsets[dot] = (dot_pos[dot] % (POS_PER_PIXEL));
    calculate_overtakes(dot);
    dot_pos[dot] += dot_speeds[dot];
    if(dot_age[dot] < 255) dot_age[dot]++;

    if(led+1 < STRIP_LENGTH) {
      CRGB old_color = leds[led];
      uint8_t old_r = old_color.r;
      uint8_t old_g = old_color.g;
      uint8_t old_b = old_color.b;
      uint8_t dot_r = dot_colors[dot].r;
      uint8_t dot_g = dot_colors[dot].g;
      uint8_t dot_b = dot_colors[dot].b;
      
      uint16_t r = old_r + dot_r * (dot_age[dot]/255.0) * (POS_PER_PIXEL - leds_offsets[dot])/POS_PER_PIXEL;
      uint16_t g = old_g + dot_g * (dot_age[dot]/255.0) * (POS_PER_PIXEL - leds_offsets[dot])/POS_PER_PIXEL;
      uint16_t b = old_b + dot_b * (dot_age[dot]/255.0) * (POS_PER_PIXEL - leds_offsets[dot])/POS_PER_PIXEL;
      leds[led+1] = CRGB( 
        min(r, 255),
        min(g, 255), 
        min(b, 255)
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
