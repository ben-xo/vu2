/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include <Arduino.h>
#include "config.h"
#include "render.h"
#include "sampler.h"
#include "renderheap.h"

#ifndef DEBUG_ONLY

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


renderheap_t r;

uint8_t static gamma8(uint8_t x)  {
  return pgm_read_byte(&_gammaTable[x]); // 0-255 in, 0-255 out
}

void setup_render() {
  // Initialize all pixels to 'off'
  // FastLED.clear();
  // FastLED.show();
  setup_attract();
}

static void generate_sparkle_table(uint8_t* random_table) {
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


// hues which range to opposites on the colour wheel, which opposites slowly changing
// saturates to which on beat; beat is a multiplier which fade
// 
void render_vu_with_beat_strobe(uint8_t peakToPeak, bool is_beat, bool is_beat_2) {
  uint8_t hue = r.rvuwbs.hue;
  uint8_t beat_offset = r.rvuwbs.beat_offset;

  if(is_beat_2) {
    beat_offset = 255; 
  } else {
    beat_offset >>= 1;
  }

  CRGB beat_color = CRGB(beat_offset, beat_offset, beat_offset);
  
  CRGB led_buffer[STRIP_LENGTH];
  
  CHSV end1_hsv = CHSV(hue,    255,255);
  CHSV end2_hsv = CHSV(hue+85,255,255);
  hsv2rgb_rainbow(end1_hsv, led_buffer[0]);
  hsv2rgb_rainbow(end2_hsv, led_buffer[STRIP_LENGTH-1]);
  fill_gradient_RGB(led_buffer, STRIP_LENGTH, led_buffer[0], led_buffer[STRIP_LENGTH-1]);

  uint8_t vu_led = map8(peakToPeak, 0, STRIP_LENGTH);

  uint8_t i;
  for(i = 0; i < vu_led; i++) {
    leds[i] = led_buffer[i] + beat_color;
  }
  for(; i < STRIP_LENGTH; i++) {
    fade_pixel_fast(i);
  }

  if(is_beat) hue++;

  r.rvuwbs.hue = hue;
  r.rvuwbs.beat_offset = beat_offset;
}

void render_vu_plus_beat_end(unsigned int peakToPeak, bool is_beat, bool do_fade) {
    uint8_t beat_brightness = r.rvupbi.beat_brightness;
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
    r.rvupbi.beat_brightness = beat_brightness;
}

void render_vu_plus_beat_interleave() {
  uint8_t beat_brightness = r.rvupbi.beat_brightness;
  uint8_t adjPeak = gamma8(F.vu_width);
  uint8_t led = map8(adjPeak, 0, STRIP_LENGTH);
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
        if (j > led) {
          leds[j] = Wheel(color);
        } else {
          fade_pixel_slow(j);
        }
      } else {
        fade_pixel_fast(j);
      }
    } else {
      if(F.is_beat_1 && random8() < 10) {
      // beats
        leds[j].setRGB(beat_brightness,beat_brightness,beat_brightness);
      } else {
        fade_pixel_fast(j);
      }      
    }
  }
  r.rvupbi.beat_brightness = beat_brightness;
}


void render_sparkles() {
    const CRGB SILVER(0xFF, 0xFF, 0xFF);
    const CRGB GOLD(0xFF, 0xFF, 0x77);
    const CRGB DARK_SILVER(0x7F, 0x7F, 0x7F);
    const CRGB DARK_GOLD(0x7F, 0x7F, 0x37);

    uint8_t adjPeak = qsub8(F.vu_width, 2); // if it's close to 0, make it 0, so it doesn't flicker
    uint8_t index = map8(adjPeak>>2, 0, STRIP_LENGTH/4);
    uint8_t random_table[STRIP_LENGTH];

    // even though strictly speaking we don't need to generate the table if index is < 1,
    // we do it anyway because it keeps the frame rate consistent.
    generate_sparkle_table(random_table);

    CRGB gold   = F.is_beat_1 ? GOLD   : DARK_GOLD;
    CRGB silver = F.is_beat_1 ? SILVER : DARK_SILVER;

    for (uint8_t j = 0; j < index; j++) {
      leds[random_table[j]] = j%2 ? gold : silver;
    }
    
    // fade the rest!
    for (uint8_t j = index; j < STRIP_LENGTH; j++) {
#     ifdef FRAME_RATE_LIMIT
      fade_pixel_fast(random_table[j]);
#     else
      fade_pixel_slow(random_table[j]);
#     endif
    }
}

// Manually unrolled version seems to give better ASM code...
void render_combo_samples_with_beat(uint8_t sample_ptr, uint16_t sample_sum) {
  uint8_t dc_offset = sample_sum/SAMP_BUFF_LEN;
  for (uint8_t j = 0; j < STRIP_LENGTH; j++) {

    uint8_t r = qsub8(sampler.samples[(sample_ptr + j*1) % SAMP_BUFF_LEN], dc_offset);
    uint8_t g = qsub8(sampler.samples[(sample_ptr + j*2) % SAMP_BUFF_LEN], dc_offset);
    uint8_t b = qsub8(sampler.samples[(sample_ptr + j*3) % SAMP_BUFF_LEN], dc_offset);

    bool is_beat = get_beat_at((sample_ptr + j*2) % SAMP_BUFF_LEN);

    if(is_beat && F.is_beat_2) {
      // V1
      leds[j].setRGB(r,g,b);
    } else if(is_beat) {
      // V2
      leds[j].setRGB(0,g,b);
    } else if(F.is_beat_2) {
      // V3
      leds[j].setRGB(r,0,b);
    } else {
      // V4
      leds[j].setRGB(0,0,b);
    }
  }
}

/**
 * FastLED is doing too much work here - and we can't therefore have 4 binsin8s in a single frame.
 * So, instead, we'll take advantage of the fact that the BPM is fixed (and irrelevant) and just do stuff using the frame counter instead
 */
uint8_t frame_beatsin8( uint8_t beat, uint8_t phase_offset = 0)
{
    uint8_t beatsin = sin8( beat + phase_offset);
    return beatsin;
}

void render_beat_line() {
    uint8_t phase = r.rbl.phase;
    uint8_t reverse_speed = 2; // range 2 to 5
    uint8_t j=0,k=0,l=0,m=0;

    if(F.is_beat_1) {
      phase -= scale8(F.vu_width, 32);
    } else {
      phase += scale8(F.vu_width, 64)+1;
    }

    uint32_t now = millis();
    uint32_t now2 = now;
    uint8_t brightness = qadd8(F.vu_width,128);

    for(uint8_t p = 0; p < STRIP_LENGTH; p++)
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

      now  -= 1; // iteratively decrement rather than calculating it by subtracting p on every loop. 
      now2 -= (1<<8);
      uint8_t beat1 = ((now)  * (30) * 280) >> 16;
      uint8_t beat2 = ((now2) * (30) * 280) >> 16;

      uint8_t sine1 = frame_beatsin8 (beat1, j+phase);
      uint8_t sine2 = frame_beatsin8 (beat1, k+phase);
      uint8_t sine3 = frame_beatsin8 (beat1, l+phase);
      uint8_t sine4 = frame_beatsin8 (beat2, phase);
      leds[p].setRGB(scale8(sine1,sine4), scale8(sine2,sine4), scale8(sine3,sine4));
      leds[p].nscale8(brightness);
      if(F.is_beat_1) {
        j -= 7;
        k -= 13;
        l -= 17;
      } else {
        j += 1; // these offsets make the colours do interesting bands
        k += 3;
        l += 5;
      }
    }
    r.rbl.phase = phase;
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
void render_bar_segments(unsigned int peakToPeak, bool is_beat) {
//    unsigned int brightness = peakToPeak / 4;
    uint8_t last_pattern;
        
    for (uint8_t j = 0; j < STRIP_LENGTH; j++)
    {
      CRGB color = Wheel(j+(bar_segment_pattern*16));
      if(_in_current_bar_segment(j)) {
        leds[j].setRGB(color.r/4*3+peakToPeak,color.g/4*3+peakToPeak,color.b/4*3+peakToPeak);
      } else {
        fade_pixel(j);
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

void render_double_vu(uint8_t peakToPeak, bool is_beat, bool is_beat_2) {
    uint8_t color=0;
    CRGB crgb_color;
    // 2 "pixels" "below" the strip, to exclude the noise floor from the VU
    uint8_t adjPeak = gamma8(peakToPeak);
    uint8_t led = map8(adjPeak, 0, STRIP_LENGTH/4);

    bool was_beat_2 = r.rdv.was_beat_2; 
    uint8_t fade_type = r.rdv.fade_type;
    
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
      else {
        if(is_beat) {
          fade_pixel_fast(j);
          fade_pixel_slow((STRIP_LENGTH/2)+j);
          fade_pixel_slow((STRIP_LENGTH/2)-j-1);
          fade_pixel_fast((STRIP_LENGTH  )-j-1);
        } else {
          fade_pixel(j);
          fade_pixel((STRIP_LENGTH/2)+j);
          fade_pixel((STRIP_LENGTH/2)-j-1);
          fade_pixel((STRIP_LENGTH  )-j-1);          
        }
      }
    }
    r.rdv.was_beat_2 = was_beat_2;
    r.rdv.fade_type = fade_type;
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

void render_black() {
    for (int j = STRIP_LENGTH - 1; j >= 0; j--) {
      leds[j].setRGB(0,0,0);
    }  
}

// void colorWipe(CRGB c, uint8_t wait) {
//   for (uint8_t i = 0; i < STRIP_LENGTH; i++) {
//     leds[i] = c;
//     FastLED.show();
//     FastLED.delay(wait);
//   }
// }

// // from strandtest example.
// void rainbowCycle(uint8_t wait) {
//   uint16_t i, j;

//   for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
//     for(i=0; i< STRIP_LENGTH; i++) {
//       leds[i] = Wheel(((i * 256 / STRIP_LENGTH) + j) & 255);
//     }
//     FastLED.show();
//     FastLED.delay(wait);
//   }
// }

static uint8_t current_pos = STRIP_LENGTH/2; // start in center
#define HALF_CENTER = (STRIP_LENGTH/4) // one quarter of the way down the strip (which quarter flips on the beat)
void render_beat_bounce_flip(bool is_beat, uint8_t peakToPeak, uint8_t sample_ptr, uint8_t min_vu, uint8_t max_vu) {
  uint16_t hue = r.rbbf.hue; // for colour cycle. It's a uint16 not 8 because the frame rate is so high. TODO: pass in the time and use the time
  
  bool top = r.rbbf.top;
  bool was_beat = r.rbbf.was_beat;
  uint8_t target_pos;
  uint8_t new_pos;

  // convert peakToPeak into 

  // target_pos is where the beat line is trying to get to (based on the current volume)
  target_pos = scale8(peakToPeak, STRIP_LENGTH);

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
    // this should be a saturated add, but it won't overflow because we don't really support strip lengths >100
    new_pos = (target_pos - current_pos)/8 + current_pos + 1;
    for(uint8_t i = 0; i < STRIP_LENGTH; i++)
    {
      if( i <= new_pos && i >= current_pos ) {
        leds[i] = CHSV(((uint8_t)hue)-i,255,255);
      } else {
        fade_pixel_fast(i);
      }
    }
  } else {
    new_pos = qsub8(current_pos - (current_pos - target_pos)/8, 1);
    for(uint8_t i = 0; i < STRIP_LENGTH; i++)
    {
      if( i >= new_pos && i <= current_pos ) {
        leds[i] = CHSV(((uint8_t)hue)+i,255,255);
      } else {
        fade_pixel_fast(i);
      }
    }
  }
  current_pos = new_pos;
  hue = (hue + 1) % 2048;

  r.rbbf.top = top;
  r.rbbf.was_beat = was_beat;
  r.rbbf.hue = hue;
}
 
void render(uint8_t sample_ptr, uint16_t sample_sum) {

    switch(F.mode) {
      default:
      case 0:
        render_vu_with_beat_strobe(F.vu_width, F.is_beat_1, F.is_beat_2);
        break;
      case 1:
        render_stream_pixels();
        break;
      case 2:
        render_double_vu(F.vu_width, F.is_beat_1, F.is_beat_2);
        break;
      case 3:
        render_vu_plus_beat_interleave();
        break;
      case 4:
        render_fire(F.is_beat_1, F.vu_width);
        break;
      case 5:
        render_sparkles();
        break;
      case 6:
        render_beat_line();
        break;
      case 7:
        render_bar_segments(F.vu_width, F.is_beat_1);
        break;
      case 8:
        render_combo_samples_with_beat(sample_ptr, sample_sum);
        break;
      case 9:
        render_beat_bounce_flip(F.is_beat_2, F.vu_width, sample_ptr, F.min_vu, F.min_vu);
    }
}




#endif // DEBUG_ONlY
