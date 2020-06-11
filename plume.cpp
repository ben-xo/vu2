/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "plume.h"

// fades pixels more the closer they are the start, so that peaks stay visible

/*
 * This version is commented out because map() is slow. Instead, we precalculate the plume fade in plume_map.h
 */
static void fade_pixel_plume(int pixel) {
  uint8_t fade_factor;
  if(pixel < STRIP_LENGTH >> 1) {
    fade_factor = map(pixel, 0, STRIP_LENGTH >> 1, 51, 0);  
  } else {
    fade_factor = map(pixel, STRIP_LENGTH >> 1, STRIP_LENGTH, 0, 51);  
  }
  leds[pixel].fadeLightBy(fade_factor);
}

//static void fade_pixel_plume(uint8_t pixel) {
//  if(pixel < STRIP_LENGTH/2) {
//    leds[pixel].fadeLightBy(pgm_read_byte(&fade_pixel_plume_map[pixel]));
//  } else {
//    leds[pixel].fadeLightBy(pgm_read_byte(&fade_pixel_plume_map2[pixel-STRIP_LENGTH/2]));
//  }
//}

// this effect needs to be rendered from the end of the strip backwards
static void stream_pixel(int pixel) {
  CRGB old_color[4];
  
  if(pixel > 3) {
    for (uint8_t i = 0; i<4; i++) {
      old_color[i] = leds[pixel-i];
      
      // Rotate and mask all colours at once.
      // Each of the 4 previous pixels contributes 1/4 brightness
      // so we divide each colour by 2.
      old_color[i].r = (old_color[i].r >> 2);
      old_color[i].g = (old_color[i].g >> 2);
      old_color[i].b = (old_color[i].b >> 2);
      old_color[i].r &= 0x3F;
      old_color[i].g &= 0x3F;
      old_color[i].b &= 0x3F;
    }

    leds[pixel] = old_color[0] + old_color[1] + old_color[2] + old_color[3];  
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

void render_stream_pixels(unsigned int peakToPeak, bool is_beat) {
    int led = map(peakToPeak, 0, 160, -2, STRIP_LENGTH/3*2 - 1) - 1;
    
    for (int j = STRIP_LENGTH-1; j >= 0; j--)
    {
      if(j <= led && led >= 0) {
        // set VU color up to peak
        int color = map(j, 0, STRIP_LENGTH, 0, 255);
        leds[j] = Wheel(-color);
      }
      else {
        stream_pixel(j);
        if(!is_beat) {
          fade_pixel_plume(j);
        }
     }
    }  
}

// this effect shifts colours along the strip on the beat.
void render_shoot_pixels(unsigned int peakToPeak, bool is_beat) {
    // only VU half the strip; for the effect to work it needs headroom.
    uint8_t led = map8(peakToPeak, 0, (STRIP_LENGTH >> 1) - 1);

    // have to render this one in reverse because 
    for (int j = STRIP_LENGTH - 1; j >= 0; j--)
    {
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
    }  
}
