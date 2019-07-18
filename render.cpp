
#include <Arduino.h>
#include "config.h"
#include "render.h"

#ifndef DEBUG_ONLY

#define SILVER CRGB(0xFF, 0xFF, 0xFF)
#define GOLD CRGB(0xFF, 0xFF, 0x77)

#define POS_PER_PIXEL (32768 / STRIP_LENGTH) // ratio of attract mode pixel-positions to actual LEDs
#define VU_PER_PIXEL (256 / STRIP_LENGTH)

uint8_t static random_table[STRIP_LENGTH];
uint8_t static maximum = 255;
uint8_t static phase = 0;
static CRGB dot_colors[ATTRACT_MODE_DOTS];
static uint32_t dot_pos[ATTRACT_MODE_DOTS];
static uint16_t dot_speeds[ATTRACT_MODE_DOTS];
static uint8_t dot_age[ATTRACT_MODE_DOTS];

static const uint8_t PROGMEM _gammaTable[256] = {
  0,   3,   5,   7,   9,  11,  13,  14,  16,  18,  19,  21,  22,  24,  25,  26,
 28,  29,  31,  32,  33,  35,  36,  37,  39,  40,  41,  42,  44,  45,  46,  47,
 48,  50,  51,  52,  53,  54,  56,  57,  58,  59,  60,  61,  63,  64,  65,  66,
 67,  68,  69,  70,  71,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
 84,  85,  86,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100,
101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
117, 118, 119, 120, 121, 122, 123, 123, 124, 125, 126, 127, 128, 129, 130, 131,
132, 133, 134, 135, 136, 137, 138, 139, 140, 140, 141, 142, 143, 144, 145, 146,
147, 148, 149, 150, 151, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161,
161, 162, 163, 164, 165, 166, 167, 168, 169, 169, 170, 171, 172, 173, 174, 175,
176, 177, 177, 178, 179, 180, 181, 182, 183, 183, 184, 185, 186, 187, 188, 189,
190, 190, 191, 192, 193, 194, 195, 196, 196, 197, 198, 199, 200, 201, 202, 202,
203, 204, 205, 206, 207, 207, 208, 209, 210, 211, 212, 212, 213, 214, 215, 216,
217, 217, 218, 219, 220, 221, 222, 222, 223, 224, 225, 226, 227, 227, 228, 229,
230, 231, 232, 232, 233, 234, 235, 236, 236, 237, 238, 239, 240, 240, 241, 242,
243, 244, 245, 245, 246, 247, 248, 249, 249, 250, 251, 252, 253, 253, 254, 255};

uint8_t static gamma8(uint8_t x)  {
  return pgm_read_byte(&_gammaTable[x]); // 0-255 in, 0-255 out
}


CRGB Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

static void make_new_dot(uint8_t dot) {
    dot_colors[dot] = Wheel(random8());
    dot_pos[dot] = random16(10000); // 32768 * 0.2 (so pixels dont start at the very end of the strip)
    dot_speeds[dot] = random16(POS_PER_PIXEL/17,POS_PER_PIXEL/5);
    dot_age[dot] = 0;
}

CRGB Wheel2(byte WheelPos) {
  // 0 is blue (0,0,255)
  // 255 is yellow (255,127,0)
  CRGB col;
  col.r = WheelPos;
  col.g = WheelPos >> 1;
  col.b = 255-WheelPos;
  return col;
}

void setup_render() {
  // Initialize all pixels to 'off'
  FastLED.clear();
  FastLED.show();
  for(uint8_t i = 0; i < ATTRACT_MODE_DOTS; i++) {
    make_new_dot(i);
  }
}

CRGB Wheel3(byte WheelPos) {
  CRGB col;
  col.r = 0;
  col.g = 128-WheelPos > 0 ? 128-WheelPos : 0;
  col.b = WheelPos > 128 ? 128 : WheelPos;
  return col;
}

CRGB Wheel_Purple_Yellow(byte WheelPos) {
  // 0 is purple (63,0,255)
  // 255 is yellow (255,127,0)
  CRGB col;
  col.r = map(WheelPos,0,255,63,255);
  col.g = WheelPos >> 2;
  col.b = 255-WheelPos;
  return col;
}

static inline uint8_t ssub8(uint8_t a, uint8_t b) {
  uint8_t c = a - b;
  if (c > a) 
    return 0;
  return c;
}

static void fade_pixel(int pixel) {
  uint8_t r = leds[pixel].r;
  uint8_t g = leds[pixel].g;
  uint8_t b = leds[pixel].b;
  // this is equivalent to r*0.875
  leds[pixel] = CRGB(ssub8(r, (r>>3)+1), ssub8(g, (g>>3)+1), ssub8(b, (b>>3)+1));
}

static void fade_pixel_slow(int pixel) {
  uint8_t r = leds[pixel].r;
  uint8_t g = leds[pixel].g;
  uint8_t b = leds[pixel].b;
  // this is equivalent to r*0.9375
  leds[pixel] = CRGB(ssub8(r, (r>>4)+1), ssub8(g, (g>>4)+1), ssub8(b, (b>>4)+1));
}

static void fade_pixel_fast(int pixel) {
  uint8_t r = leds[pixel].r;
  uint8_t g = leds[pixel].g;
  uint8_t b = leds[pixel].b;
  leds[pixel] = CRGB(ssub8(r, (r>>2)+1), ssub8(g, (g>>2)+1), ssub8(b, (b>>2)+1));
  // this is equivalent to r*0.75
}

// fades pixels more the closer they are the start, so that peaks stay visible
static void fade_pixel_plume(int pixel) {
  uint8_t fade_factor;
  if(pixel < STRIP_LENGTH >> 1) {
    fade_factor = map(pixel, 0, STRIP_LENGTH >> 1, 51, 0);  
  } else {
    fade_factor = map(pixel, STRIP_LENGTH >> 1, STRIP_LENGTH, 0, 51);  
  }
  leds[pixel].fadeLightBy(fade_factor);
}

static void generate_sparkle_table() {
  int i;
  
  for (i = 0; i < STRIP_LENGTH; i++) {
    random_table[i] = i;
  }

  // shuffle!
  // we only shuffle HALF the table, because render_sparkle
  // only 
  for (i = 0; i < STRIP_LENGTH / 2; i++)
  {
      size_t j = random8(STRIP_LENGTH - i + 1);
    
      int t = random_table[i];
      random_table[i] = random_table[j];
      random_table[j] = t;
  }  
}



// this effect needs to be rendered from the end of the strip backwards
void stream_pixel(int pixel) {
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
void shoot_pixel(int pixel) {
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
    fade_pixel(pixel);
  }

  leds[pixel] = color;  
}

void render_vu_plus_beat_end(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    static uint8_t beat_brightness;
    uint8_t adjPeak = gamma8(peakToPeak);
    int led = map8(adjPeak, 0, STRIP_LENGTH);
    uint8_t color = 0;
    beat_brightness = qadd8(beat_brightness/2, adjPeak);
    
    for (uint8_t j = 0; j < STRIP_LENGTH; j++)
    {
      // 2 "pixels" "below" the strip, to exclude the noise floor from the VU
        if(j <= led && led >= 0) {
          // set VU color up to peak
          color += VU_PER_PIXEL + VU_PER_PIXEL; // double it because half the range is beat flash
          leds[j] = Wheel(color);
        }
        else if(j >= STRIP_LENGTH/2 && j < STRIP_LENGTH && is_beat) {
          leds[j].setRGB(beat_brightness,beat_brightness,beat_brightness);
        }
        else if(do_fade) {
          if( j >= STRIP_LENGTH/2 ) {
            fade_pixel_slow(j);
          } else {
            fade_pixel_fast(j);
          }
        }
    }  
}

void render_vu_plus_beat_interleave(uint8_t peakToPeak, bool is_beat, bool do_fade) {
  static uint8_t beat_brightness;
  uint8_t adjPeak = gamma8(peakToPeak);
  int led = map8(adjPeak, 0, STRIP_LENGTH);
  uint8_t color = 0;
  beat_brightness = qadd8(beat_brightness/2, adjPeak);
  
  for (uint8_t j = 0; j < STRIP_LENGTH; j++)
  {
  // 2 "pixels" "below" the strip, to exclude the noise floor from the VU
    if(j % 2) {
      // VU
      if(j <= led*2) {
        // set VU color up to peak
        color -= VU_PER_PIXEL + VU_PER_PIXEL; // double it because half the range is beat flash
        leds[j] = Wheel(color);
      } else {
        if(do_fade) {
          fade_pixel_fast(j);
        }
      }
    } else {
      if(is_beat) {
      // beats
        leds[j].setRGB(beat_brightness,beat_brightness,beat_brightness);
      } else {
        if(do_fade) {
          fade_pixel_slow(j);
        }    
      }      
    }
  }
}

void render_stream_pixels(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    int led = map(peakToPeak, 0, 160, -2, STRIP_LENGTH/3*2 - 1) - 1;
    
    for (int j = STRIP_LENGTH-1; j >= 0; j--)
    {
      if(j <= led && led >= 0) {
        // set VU color up to peak
        int color = map(j, 0, STRIP_LENGTH, 0, 255);
        leds[j] = Wheel(color);
      }
      else {
        stream_pixel(j);
         if(!is_beat && do_fade) {
          fade_pixel_plume(j);
        }
     }
    }  
}

// this effect shifts colours along the strip on the beat.
void render_shoot_pixels(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    // only VU half the strip; for the effect to work it needs headroom.
    uint8_t led = map(peakToPeak, 0, 128, 0, (STRIP_LENGTH >> 1) - 1);
    
    for (int j = STRIP_LENGTH - 1; j >= 0; j--)
    {
      if(j <= led) {
        // set VU color up to peak
        int color = map(j, 0, STRIP_LENGTH >> 2, 0, 255);
        leds[j] = Wheel_Purple_Yellow(color);
      }
      else {
        shoot_pixel(j);
        if(!is_beat && do_fade) {
          fade_pixel_plume(j);
        }
      }
    }  
}

void render_sparkles(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    if(do_fade) {
      for (uint8_t j = 0; j < STRIP_LENGTH; j++)
      {
        fade_pixel(j);
      }
    }
    int index = map(peakToPeak, 0, maximum, -2, STRIP_LENGTH/2 );
    if(index >= 0) {
      generate_sparkle_table();
      for (uint8_t j = is_beat?0:(index/4); j <= index; j++) {
        leds[random_table[j]] = j%2 ? GOLD : SILVER;
      }
    }
}

// Manually unrolled version seems to give better ASM code...
void render_combo_samples_with_beat(bool is_beat, bool is_beat_2, uint8_t sample_ptr) {
  for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
    uint8_t r = samples[(sample_ptr + j*1) % SAMP_BUFF_LEN];
    uint8_t g = samples[(sample_ptr + j*3) % SAMP_BUFF_LEN];
    uint8_t b = samples[(sample_ptr + j*5) % SAMP_BUFF_LEN];
    if(is_beat && is_beat_2) {
      // V1
      leds[j].setRGB(r,g,b);
    } else if(is_beat) {
    // V2
      leds[j].setRGB(0,g,b);
    } else if(is_beat_2) {
    // V3
      leds[j].setRGB(r,0,b);
    } else {
    // V4
      leds[j].setRGB(0,0,b);
    }
  }
}

void render_beat_line(unsigned int peakToPeak, bool is_beat, bool is_beat_2) {
    uint8_t reverse_speed = (peakToPeak >> 6) + 2; // range 2 to 5
    uint16_t p,j,k,l;
    p=j=k=l=0;
    while(p < STRIP_LENGTH)
    {
      // shift all the pixels along
//      uint16_t sine1 = sin8((uint8_t)(j+phase));
//      uint16_t sine2 = sin8((uint8_t)(k+phase));
//      uint16_t sine3 = sin8((uint8_t)(l+phase));
//      if(!is_beat && !is_beat_2) {
//        // sine1, 2 and 3 are really RGB values. Adjustment is really a base white.
//        uint16_t adjustment = peakToPeak / 4 * 3;
//        sine1 = adjustment + (sine1 / 4); if(sine1 > 255) sine1 = 255; // saturate
//        sine2 = adjustment + (sine2 / 4); if(sine2 > 255) sine2 = 255; // saturate
//        sine3 = adjustment + (sine3 / 4); if(sine3 > 255) sine3 = 255; // saturate
//      }

      uint8_t sine1 = beatsin8 (peakToPeak>>4, 0, 255, 0, p+j+phase);
      uint8_t sine2 = beatsin8 (peakToPeak>>4, 0, 255, 0, p+k+phase);
      uint8_t sine3 = beatsin8 (peakToPeak>>4, 0, 255, 0, p+l+phase);
      leds[p].setRGB(sine1, sine2, sine3);
      p++;
      j += 5;
      k += 10;
      l += 15;
    }
    if(is_beat) {
      phase += reverse_speed;
      if(is_beat_2) {
        phase += reverse_speed;
      }
    } else {
      phase--;
    }
}

uint32_t static was_beat_recently_time = 0;
uint8_t static bar_segment_pattern=0;
#define BAR_PATTERNS 8
#define BAR_PATTERN_SIZE 8

static const uint8_t PROGMEM bar_patterns[] = {
  0b11110000, 0b11110000, 0b11110000, 0b11110000, 0b11110000, 0b11110000, 0b11110000, 0b11110000,
  0b11111111, 0b00000000, 0b11111111, 0b00000000, 0b11111111, 0b00000000, 0b11111111, 0b00000000,
  0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b00000000, 0b00000000,
  0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00001111, 0b00001111, 0b00001111, 0b00001111, 0b00001111, 0b00001111, 0b00001111, 0b00001111,
  0b00000000, 0b11111111, 0b00000000, 0b11111111, 0b00000000, 0b11111111, 0b00000000, 0b11111111,
  0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b00000000, 0b00000000, 0b11111111, 0b11111111,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
};
static boolean _in_current_bar_segment(uint8_t j) {
  uint16_t offset = bar_segment_pattern;
  return (pgm_read_byte(&bar_patterns[(offset*BAR_PATTERNS) + ((j / BAR_PATTERN_SIZE) % BAR_PATTERN_SIZE)]) >> (j % BAR_PATTERN_SIZE)) & 1;
}
void render_bar_segments(unsigned int peakToPeak, bool is_beat, bool do_fade) {
//    unsigned int brightness = peakToPeak / 4;
    uint8_t last_pattern;
        
    for (uint8_t j = 0; j < STRIP_LENGTH; j++)
    {
      CRGB color = Wheel(j+(bar_segment_pattern*16));
      if(_in_current_bar_segment(j)) {
        leds[j].setRGB(color.r/4*3+peakToPeak,color.g/4*3+peakToPeak,color.b/4*3+peakToPeak);
      } else {
        if(do_fade) {
          fade_pixel(j);
        }
      }
    }
    if(is_beat && millis() > was_beat_recently_time + 250) {
      was_beat_recently_time = millis();
      last_pattern = bar_segment_pattern;
      while(last_pattern == bar_segment_pattern) {
        bar_segment_pattern = random8(BAR_PATTERNS);
      }
      if(bar_segment_pattern >= BAR_PATTERNS) bar_segment_pattern=0;
    }
}

void render_double_vu(uint8_t peakToPeak, bool is_beat, bool do_fade, bool is_beat_2) {
    uint8_t color=0;
    CRGB crgb_color;
    // 2 "pixels" "below" the strip, to exclude the noise floor from the VU
    uint8_t adjPeak = gamma8(peakToPeak);
    uint8_t led = map8(adjPeak, 0, STRIP_LENGTH/2);

    static bool was_beat_2 = false;
    static uint8_t fade_type = 0;
    
    if(is_beat_2) {
      if(!was_beat_2) {
        // new fade type on each beat_2
        fade_type++;
        if (fade_type > 2) fade_type = 0;
      }
      was_beat_2 = true;
    } else {
      was_beat_2 = false;
    }
    
    for (uint8_t j = 0; j < STRIP_LENGTH/4; j++)
    {
      if(j <= led && led >= 0) {
        color += (VU_PER_PIXEL*4);
        
        // set VU color up to peak
        switch(fade_type) {
          default:
          case 0: crgb_color = Wheel(color); break;
          case 1: crgb_color = Wheel2(color); break;
          case 2: crgb_color = Wheel3(color); break;
        }
        leds[j] = crgb_color;
        leds[(STRIP_LENGTH/2)+j] = crgb_color;
        leds[(STRIP_LENGTH/2)-j-1] = crgb_color;
        leds[(STRIP_LENGTH)-j-1] = crgb_color;
      }
      else if(do_fade) {
        if(is_beat) {
          fade_pixel(j);
          fade_pixel_slow((STRIP_LENGTH/2)+j);
          fade_pixel_slow((STRIP_LENGTH/2)-j-1);
          fade_pixel((STRIP_LENGTH  )-j-1);
        } else {
          fade_pixel(j);
          fade_pixel((STRIP_LENGTH/2)+j);
          fade_pixel((STRIP_LENGTH/2)-j-1);
          fade_pixel((STRIP_LENGTH  )-j-1);          
        }
      }
    }  
}

void render_beat_flash_1_pixel(bool is_beat) {
    // THIS BIT FLASHES ONE LED SO YOU CAN SEE THE BEATS
    if(is_beat) {
      leds[0] = CRGB(127,127,127);
    } else {
      leds[0] = CRGB::Black;
    }
    for (uint8_t j = STRIP_LENGTH - 1; j >= 1; j--) {
      leds[j] = CRGB::Black;
    }  
}

void render_threshold() {
  // THIS BIT DRAWS A NUMBER IN BINARY ON TO THE STRIP
  unsigned int threshold = analogRead(THRESHOLD_INPUT);
  for(uint8_t i = 0; i < STRIP_LENGTH; i++)
  {
    if (threshold & 0x01) {
      leds[i].setRGB(127,127,127);
    } else {
      leds[i].setRGB(0,0,0);
    }
    threshold = threshold >> 1;
  }
}

void render_black() {
    for (int j = STRIP_LENGTH - 1; j >= 0; j--) {
      leds[j].setRGB(0,0,0);
    }  
}

void colorWipe(CRGB c, uint8_t wait) {
  for (uint8_t i = 0; i < STRIP_LENGTH; i++) {
    leds[i] = c;
    FastLED.show();
    delay(wait);
  }
}

// from strandtest example.
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< STRIP_LENGTH; i++) {
      leds[i] = Wheel(((i * 256 / STRIP_LENGTH) + j) & 255);
    }
    FastLED.show();
    delay(wait);
  }
}

static uint8_t current_pos = STRIP_LENGTH/2; // start in center
void render_beat_bounce_flip(bool is_beat, unsigned int peakToPeak, uint8_t sample_ptr) {  
  static bool top = true; // which half?
  static bool was_beat = false; // which half?
  uint8_t target_pos;
  uint8_t new_pos;
  if (STRIP_LENGTH >= 64) {
    target_pos = peakToPeak /2; // range 0 to 31
  }
  else if (STRIP_LENGTH >= 48 && STRIP_LENGTH < 64) {
    target_pos = (peakToPeak /2) + (peakToPeak /4); // range 0 to 23
  }
  else if (STRIP_LENGTH >= 32 && STRIP_LENGTH < 48) {
    target_pos = (peakToPeak /4); // range 0 to 15
  }

  // TODO: could easily make this interrupt driven too
  if(is_beat) {
    if(!was_beat) {
      top = !top;
    }
    was_beat = true;
  } else {
    was_beat = false;
  }

  // flip sides
  if(top) {
    target_pos = STRIP_LENGTH - target_pos;
  }

  // home in
  if(target_pos > current_pos) {
    new_pos = (target_pos - current_pos)/8 + current_pos + 1;
    for(uint8_t i = 0; i < STRIP_LENGTH; i++)
    {
      if( i <= new_pos && i >= current_pos ) {
        leds[i] = Wheel(0-i+peakToPeak);
      } else {
        fade_pixel_fast(i);
      }
    }
  } else {
    new_pos = ssub8(current_pos - (current_pos - target_pos)/8, 1);
    for(uint8_t i = 0; i < STRIP_LENGTH; i++)
    {
      if( i >= new_pos && i <= current_pos ) {
        leds[i] = Wheel(i+peakToPeak);
      } else {
        fade_pixel_fast(i);
      }
    }
  }
  current_pos = new_pos;
}
 
void render(unsigned int peakToPeak, bool is_beat, bool do_fade, byte mode, bool is_beat_2, uint8_t sample_ptr) {

    switch(mode) {
      default:
      case 0:
        render_vu_plus_beat_end(peakToPeak, is_beat, do_fade);
        break;
      case 1:
        render_shoot_pixels(peakToPeak, is_beat, do_fade);
        break;
      case 2:
        render_double_vu(peakToPeak, is_beat, do_fade, is_beat_2);
        break;
      case 3:
        render_vu_plus_beat_interleave(peakToPeak, is_beat, do_fade);
        break;
      case 4:
        render_fire(is_beat, peakToPeak);
        break;
      case 5:
        render_sparkles(peakToPeak, is_beat, do_fade);
        break;
      case 6:
        render_beat_line(peakToPeak, is_beat, is_beat_2);
        break;
      case 7:
        render_bar_segments(peakToPeak, is_beat, do_fade);
        break;
      case 8:
        render_combo_samples_with_beat(is_beat_2, is_beat, sample_ptr);
        break;
      case 9:
        render_beat_bounce_flip(is_beat, peakToPeak, sample_ptr);
    }
}

void calculate_overtakes(uint8_t dot) {
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


void do_banner() {
    // colour test
    for (uint16_t i = 0; i <= 255; i += STRIP_LENGTH/30) {
      for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
        CRGB c = Wheel((j + (i/4)) * 255 / STRIP_LENGTH);
        leds[j].setRGB(c.r * i / 255, c.g * i / 255, c.b * i / 255);
      }
      FastLED.show();
    }

    for (int16_t k = 255; k > -1; k -= STRIP_LENGTH/30) {
      for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
        CRGB c = Wheel((j + (STRIP_LENGTH-k)/4) * 255 / STRIP_LENGTH);
        leds[j].setRGB(c.r * k / 255, c.g * k / 255, c.b * k / 255);
      }
      FastLED.show();
    }
    delay(100);

    // double flash
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
        leds[j].setRGB(255,255,255);
    }
    FastLED.show();
    delay(50);

    FastLED.clear();
    FastLED.show();
    delay(50);

    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
        leds[j].setRGB(255,255,255);
    }
    FastLED.show();
    delay(50);

    FastLED.clear();
    FastLED.show();
    delay(100);
}


#endif // DEBUG_ONlY
