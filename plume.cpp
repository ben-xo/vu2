/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "plume.h"

// fades pixels more the closer they are the start, so that peaks stay visible

static void fade_pixel_plume(uint8_t pixel) {
  uint8_t fade_factor;
  if(pixel < STRIP_LENGTH >> 1) {
    fade_factor = map8(pixel * VU_PER_PIXEL, 51, 0);  
  } else {
    fade_factor = map8(pixel * VU_PER_PIXEL, 0, 51);  
  }
  leds[pixel].fadeToBlackBy(fade_factor);
}

/*
 * This version is commented out because previously map() was too slow but then I discovered FastLED
 */
//static void fade_pixel_plume(uint8_t pixel) {
//  if(pixel < STRIP_LENGTH/2) {
//    leds[pixel].fadeLightBy(pgm_read_byte(&fade_pixel_plume_map[pixel]));
//  } else {
//    leds[pixel].fadeLightBy(pgm_read_byte(&fade_pixel_plume_map2[pixel-STRIP_LENGTH/2]));
//  }
//}

// this effect needs to be rendered from the end of the strip backwards
static void stream_pixel(uint8_t pixel) {
  if(pixel > 3) {
    leds[pixel].setRGB((leds[pixel].r >> 2) + (leds[pixel-1].r >> 2) + (leds[pixel-2].r >> 2) + (leds[pixel-3].r >> 2),
                       (leds[pixel].g >> 2) + (leds[pixel-1].g >> 2) + (leds[pixel-2].g >> 2) + (leds[pixel-3].g >> 2),
                       (leds[pixel].b >> 2) + (leds[pixel-1].b >> 2) + (leds[pixel-2].b >> 2) + (leds[pixel-3].b >> 2));
  } else {
    fade_pixel(pixel);
  }

}

// like stream pixel but with a sharper fade
static void shoot_pixel(uint8_t pixel) {
  CRGB color = CRGB(0,0,0);

  if(pixel >= 4) {
    color.r  = ((leds[pixel-2].r >> 1) & 0x7F);
    color.r += ((leds[pixel-3].r >> 2) & 0x3F);
    color.r += ((leds[pixel-4].r >> 3) & 0x1F);
    color.g  = ((leds[pixel-2].g >> 1) & 0x7F);
    color.g += ((leds[pixel-3].g >> 2) & 0x3F);
    color.g += ((leds[pixel-4].g >> 3) & 0x1F);    
    color.b  = ((leds[pixel-2].b >> 1) & 0x7F);
    color.b += ((leds[pixel-3].b >> 2) & 0x3F);
    color.b += ((leds[pixel-4].b >> 3) & 0x1F);    
  } else {
    fade_pixel_slow(pixel);
  }

  leds[pixel] = color;  
}

void render_stream_pixels() {
    uint8_t led = map8(F.vu_width, 0, STRIP_LENGTH/3*2 - 1);
    led = qsub8(led, 2);

    uint8_t j = STRIP_LENGTH;
    do {
      j--;
      if(j <= led && led >= 0) {
        // set VU color up to peak
        uint8_t color = j * VU_PER_PIXEL;
        leds[j] = Wheel(-color);
      }
      else {
        stream_pixel(j);
        if(!F.is_beat_1) {
          fade_pixel_plume(j);
        }
     }
    } while(j > 0);
}

// this effect shifts colours along the strip on the beat.
void render_shoot_pixels(uint8_t peakToPeak, bool is_beat) {
    // only VU half the strip; for the effect to work it needs headroom.
    uint8_t led = map8(peakToPeak, 0, (STRIP_LENGTH >> 1) - 1);

    // have to render this one in reverse because
    uint8_t j = STRIP_LENGTH;
    do
    {
      j--;
      if(j == led) {
        // set VU color up to peak
        uint8_t color = j * VU_PER_PIXEL;
        leds[j] = Wheel_Purple_Yellow(color);
      }
      else {
        shoot_pixel(j);
        if(!is_beat) {
          fade_pixel_plume(j);
        }
      }
    } while(j > 0);
}
