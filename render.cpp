#include <Arduino.h>
#include "config.h"
#include "render.h"
#include "ultrafastneopixel.h"

#define SILVER 0xFFFFFFFF
#define GOLD 0xFFFFFF77

// modes 0 to MAX_MODE are effects
#define MAX_MODE 10
#define MAX_AUTO_MODE 8


uint8_t random_table[STRIP_LENGTH];
uint8_t maximum = 110;

void setup_render() {
  // Initialize all pixels to 'off'
  strip.begin();
  strip.clear();
  strip.show();
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

uint32_t Wheel2(byte WheelPos) {
  // 0 is blue (0,0,255)
  // 255 is yellow (255,127,0)
  return strip.Color(
    WheelPos, 
    WheelPos >> 1, 
    255-WheelPos);
}

uint32_t Wheel3(byte WheelPos) {
  return strip.Color(0, 128-WheelPos > 0 ? 128-WheelPos : 0, WheelPos > 128 ? 128 : WheelPos);
}

uint32_t Wheel_Purple_Yellow(byte WheelPos) {
  // 0 is purple (63,0,255)
  // 255 is yellow (255,127,0)
  
  return strip.Color(
    map(WheelPos,0,255,63,255),
    WheelPos >> 2,
    255-WheelPos
  );
}

void fade_pixel(int pixel) {
  uint32_t color = strip.getPixelColor(pixel);
  uint8_t r = color >> 16;
  uint8_t g = color >> 8;
  uint8_t b = color;
  strip.setPixelColor(pixel, r*0.8,g*0.8,b*0.8);
}

void fade_pixel_slow(int pixel) {
  uint32_t color = strip.getPixelColor(pixel);
  uint8_t r = color >> 16;
  uint8_t g = color >> 8;
  uint8_t b = color;
  strip.setPixelColor(pixel, r*0.95,g*0.95,b*0.95);
}

// fades pixels more the closer they are the start, so that peaks stay visible
void fade_pixel_plume(int pixel) {
  float fade_factor;
  if(pixel < STRIP_LENGTH >> 1) {
    fade_factor = map(pixel, 0, STRIP_LENGTH >> 1, 0.5, 1.0);  
  } else {
    fade_factor = map(pixel, STRIP_LENGTH >> 1, STRIP_LENGTH, 1.0, 0.5);  
  }
  uint32_t color = strip.getPixelColor(pixel);
  uint8_t r = color >> 16;
  uint8_t g = color >> 8;
  uint8_t b = color;
  strip.setPixelColor(pixel, r*fade_factor, g*fade_factor, b*fade_factor);
}

void generate_sparkle_table() {
  int i;
  
  for (i = 0; i < STRIP_LENGTH; i++) {
    random_table[i] = i;
  }

  // shuffle!
  // we only shuffle HALF the table, because render_sparkle
  // only 
  for (i = 0; i < STRIP_LENGTH / 2; i++)
  {
      size_t j = random(0, STRIP_LENGTH - i);
    
      int t = random_table[i];
      random_table[i] = random_table[j];
      random_table[j] = t;
  }  
}



// this effect needs to be rendered from the end of the strip backwards
void stream_pixel(int pixel) {
  uint32_t old_color[4];
  
  if(pixel > 3) {
    for (uint8_t i = 0; i<4; i++) {
      old_color[i] = strip.getPixelColor(pixel-i);
      
      // Rotate and mask all colours at once.
      // Each of the 4 previous pixels contributes 1/4 brightness
      // so we divide each colour by 2.
      old_color[i] = (old_color[i] >> 2) & 0x3F3F3F3F;
    }

    strip.setPixelColor(pixel, old_color[0] + old_color[1] + old_color[2] + old_color[3]);  
  } else {
    fade_pixel(pixel);
  }
}

// like stream pixel but with a sharper fade
void shoot_pixel(int pixel) {
  uint32_t color = 0;
  
  if(pixel >= 4) {
    color  = (strip.getPixelColor(pixel-2) >> 1) & 0x7F7F7F7F;
    color += (strip.getPixelColor(pixel-3) >> 2) & 0x3F3F3F3F;
    color += (strip.getPixelColor(pixel-4) >> 3) & 0x1F1F1F1F;    
  } else {
    fade_pixel(pixel);
  }

  strip.setPixelColor(pixel, color);  
}

void render_vu_plus_beat_end(unsigned int peakToPeak, bool is_beat, bool do_fade, unsigned int lpvu, unsigned int hpvu) {
    int led = map(peakToPeak, 0, maximum, -2, STRIP_LENGTH - 1) - 1;
    int beat_brightness = map(peakToPeak, 0, maximum, 0, 255);
    int bias = lpvu;
    
    for (int j = STRIP_LENGTH - 1; j >= 0; j--)
    {
      // 2 "pixels" "below" the strip, to exclude the noise floor from the VU
        if(j <= led && led >= 0) {
          // set VU color up to peak
          int color = map(j, 0, STRIP_LENGTH, 0, 255);
          strip.setPixelColor(j, Wheel((color-bias)%256));
        }
        else if(j >= STRIP_LENGTH/2 && j < STRIP_LENGTH && is_beat) {
          strip.setPixelColor(j, beat_brightness,beat_brightness,beat_brightness);
        }
        else if(do_fade) {
          fade_pixel(j);
        }
    }  
}

void render_stream_pixels(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    int led = map(peakToPeak, 0, maximum, -2, STRIP_LENGTH/2 - 1) - 1;
    
    for (int j = STRIP_LENGTH-1; j >= 0; j--)
    {
      if(j <= led && led >= 0) {
        // set VU color up to peak
        int color = map(j, 0, STRIP_LENGTH, 0, 255);
        strip.setPixelColor(j, Wheel(color));
      }
      else {
        stream_pixel(j);
      }
    }  
}

// this effect shifts colours along the strip on the beat.
void render_shoot_pixels(unsigned int peakToPeak, bool is_beat, bool do_fade, unsigned int lpvu, unsigned int hpvu) {
    // only VU half the strip; for the effect to work it needs headroom.
    uint8_t led = map(peakToPeak, 0, maximum, -2, (STRIP_LENGTH >> 1) - 1) - 1;
    
    for (int j = STRIP_LENGTH - 1; j >= 0; j--)
    {
      if(j <= led && led >= 0) {
        // set VU color up to peak
        int color = map(j, 0, STRIP_LENGTH >> 2, 0, 255);
        strip.setPixelColor(j, Wheel_Purple_Yellow(color));
      }
      else {
        shoot_pixel(j);
        if(!is_beat && do_fade) {
          fade_pixel_plume(j);
        }
      }
    }  
}

void render_vu_plus_beat_interleave(unsigned int peakToPeak, bool is_beat, bool do_fade, unsigned int lpvu, unsigned int hpvu) {
  uint8_t led = map(peakToPeak, 0, maximum, -2, STRIP_LENGTH - 1) - 1;
  uint8_t beat_brightness = map(peakToPeak, 0, maximum, 0, 255);
  unsigned int bias = lpvu;

  for (int j = 0; j < STRIP_LENGTH; j++ ) {
    if(j % 2) {
    
      // VU
      if(j <= led && led >= 0) {
        // set VU color up to peak
        uint8_t color = map(j, 0, STRIP_LENGTH, 0, 255);
        strip.setPixelColor(j, Wheel((color-bias)%256));
      } 
    } else if(is_beat) {
    // beats
      strip.setPixelColor(j, beat_brightness,beat_brightness,beat_brightness);
    }
     
    if(do_fade) {
      fade_pixel(j);
    }
  }
}

void render_sparkles(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    if(do_fade) {
      for (uint8_t j = 0; j < STRIP_LENGTH; j++)
      {
        is_beat ? fade_pixel_slow(j) : fade_pixel(j);
      }
    }
    int index = map(peakToPeak, 0, maximum, -2, STRIP_LENGTH/3 );
    if(index >= 0) {
      generate_sparkle_table();
      for (uint8_t j = 0; j <= index; j++) {
        strip.setPixelColor(random_table[j], j%2 ? GOLD : SILVER);
      }
    }
}

void render_beat_line(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    int color = map(peakToPeak, 0, maximum, 0, 255);
    for (uint8_t j = STRIP_LENGTH - 1; j > 0; j--)
    {
      // shift all the pixels along
      strip.setPixelColor(j, strip.getPixelColor(j-1));
    }
    if(is_beat) {
      strip.setPixelColor(0, 255, 255, 255);
    } else {
      strip.setPixelColor(0, color >> 2, color >> 2, color >> 2);
    }
}

uint32_t was_beat_recently_time = 0;
uint32_t last_bar_color = 0;
int bar_segment_pattern=0;
#define BAR_PATTERN_SIZE 8
long long unsigned bar_patterns[] = {
  0b1111000011110000111100001111000011110000111100001111000011110000,
  0b1111111100000000111111110000000011111111000000001111111100000000,
  0b1111111111111111000000000000000011111111111111110000000000000000,
  0b1111111111111111111111111111111100000000000000000000000000000000,
  0b0000111100001111000011110000111100001111000011110000111100001111,
  0b0000000011111111000000001111111100000000111111110000000011111111,
  0b0000000000000000111111111111111100000000000000001111111111111111,
  0b0000000000000000000000000000000011111111111111111111111111111111,
};
boolean _in_current_bar_segment(int j) {
  int offset = bar_segment_pattern;
  return (bar_patterns[offset] >> j) & 0b0000000000000000000000000000000000000000000000000000000000000001;
}
void render_bar_segments(unsigned int peakToPeak, bool is_beat, bool do_fade, unsigned int lpvu) {
    uint32_t new_bar_color = Wheel((map(peakToPeak, 0, maximum/2, 0, 255)+(bar_segment_pattern << 4))%256);
    uint32_t color = (((new_bar_color) >> 1) & 0x7F7F7F7F) + ((last_bar_color >> 1) & 0x7F7F7F7F);
    
    for (uint8_t j = STRIP_LENGTH - 1; j > 0; j--)
    {
      if(_in_current_bar_segment(j)) {
        strip.setPixelColor(j, color);
        last_bar_color = color;
      } else {
        if(do_fade) {
          fade_pixel(j);
        }
      }
     }
    if(is_beat && millis() > was_beat_recently_time + 250) {
      was_beat_recently_time = millis();
      bar_segment_pattern = random(0,BAR_PATTERN_SIZE);
      if(bar_segment_pattern > 7) bar_segment_pattern=0;
    }
}

void render_double_vu(unsigned int peakToPeak, bool is_beat, bool do_fade, char fade_type, unsigned int lpvu, unsigned int hpvu) {
    uint32_t color;
    // 2 "pixels" "below" the strip, to exclude the noise floor from the VU
    int led = map(peakToPeak, 0, maximum, -2, STRIP_LENGTH/2);
    int bias = lpvu;
    
    for (uint8_t j = 0; j < STRIP_LENGTH/4; j++)
    {
      if(j <= led && led >= 0) {
        
        // set VU color up to peak
        color = map(j, 0, STRIP_LENGTH/4, 0, 255);
        switch(fade_type) {
          default:
          case 0: color = Wheel((color+bias)%256); break;
          case 1: color = Wheel2((color+bias)%256); break;
          case 2: color = Wheel3((color+bias)%256); break;
        }
        strip.setPixelColor(j, color);
        strip.setPixelColor((STRIP_LENGTH/2)+j, color);
        
        // set VU color up to peak
        color = map(j, 0, STRIP_LENGTH/4, 255, 0);
        switch(fade_type) {
          default:
          case 0: color = Wheel((color+bias)%256); break;
          case 1: color = Wheel2((color+bias)%256); break;
          case 2: color = Wheel3((color+bias)%256); break;
        }
        strip.setPixelColor((STRIP_LENGTH/2)-j-1, color);
        strip.setPixelColor((STRIP_LENGTH)-j-1, color);
      }
      else if(do_fade) {
        fade_pixel(j);
        fade_pixel((STRIP_LENGTH/2)+j);
        fade_pixel((STRIP_LENGTH/2)-j-1);
        fade_pixel((STRIP_LENGTH  )-j-1);
      }
    }  
}

void render_beat_flash_1_pixel(bool is_beat) {
    // THIS BIT FLASHES ONE LED SO YOU CAN SEE THE BEATS
    if(is_beat) {
      strip.setPixelColor(0, strip.Color(127,127,127));
    } else {
      strip.setPixelColor(0, 0);
    }
    for (uint8_t j = STRIP_LENGTH - 1; j >= 1; j--) {
      strip.setPixelColor(j, 0);
    }  
}

void render_threshold() {
  // THIS BIT DRAWS A NUMBER IN BINARY ON TO THE STRIP
  unsigned int threshold = analogRead(THRESHOLD_INPUT);
  for(uint8_t i = 0; i < STRIP_LENGTH; i++)
  {
    if (threshold & 0x01) {
      strip.setPixelColor(i, strip.Color(127,127,127));
    } else {
      strip.setPixelColor(i, 0);
    }
    threshold = threshold >> 1;
  }
}

void render_black() {
    for (int j = STRIP_LENGTH - 1; j >= 0; j--) {
      strip.setPixelColor(j, 0);
    }  
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint8_t i = 0; i < STRIP_LENGTH; i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// from strandtest example.
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< STRIP_LENGTH; i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / STRIP_LENGTH) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void render(unsigned int peakToPeak, bool is_beat, bool do_fade, char mode, unsigned int lpvu, unsigned int hpvu) {

    switch(mode) {
      case 0:
        render_vu_plus_beat_end(peakToPeak, is_beat, do_fade, lpvu, hpvu);
        break;
      case 1:
        render_shoot_pixels(peakToPeak, is_beat, do_fade, lpvu, hpvu);
        break;
      case 2:
        render_double_vu(peakToPeak, is_beat, do_fade, 0, lpvu, hpvu);
        break;
      case 3:
        render_vu_plus_beat_interleave(peakToPeak, is_beat, do_fade, lpvu, hpvu);
        break;
      case 4:
        render_double_vu(peakToPeak, is_beat, do_fade, 1, lpvu, hpvu);
        break;
      case 5:
        render_stream_pixels(peakToPeak, is_beat, do_fade);
        break;
      case 6:
        render_sparkles(peakToPeak, is_beat, do_fade);
        break;
      case 7:
        render_double_vu(peakToPeak, is_beat, do_fade, 2, lpvu, hpvu);
        break;
      case 8:
        render_beat_line(peakToPeak, is_beat, do_fade);
        break;
      case 9:
        render_bar_segments(peakToPeak, is_beat, do_fade, lpvu);
        break;
        
      case 10:
        render_beat_flash_1_pixel(is_beat);
        break;
      case 11:
        render_threshold();
        break;
      default:
        render_black();
        break;
    }
}
