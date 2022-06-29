/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 *
 * Most of this file is adapted from FastLED DemoReel100.ino by Mark Kriegsman.
 */

#include "demo.h"
#include "loop.h"


#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(STRIP_LENGTH) ] += CRGB::White;
  }
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, STRIP_LENGTH, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, STRIP_LENGTH, 10);
  int pos = random16(STRIP_LENGTH);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, STRIP_LENGTH, 20);
  int pos = beatsin16( 13, 0, STRIP_LENGTH-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, STRIP_LENGTH, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, STRIP_LENGTH-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < STRIP_LENGTH; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void setup_demo() {
  Serial.begin(2000000);
} 

static void _demo_loop(const uint8_t start_sober)
{
  uint8_t pushed = NO_PUSH;
  uint8_t mode = start_sober ? 13 : 0;
  uint8_t render_mode = start_sober;

  // demo mode changes pattern every 10 seconds, but increments hue every 20ms
  uint8_t gHue_divider = 20 * FPS / 1000; // every 20ms = every 3 frames at 150FPS
  uint8_t gHue_counter = gHue_divider;
  uint16_t demo_divider = 10 * FPS; // every 10s = every 1500 frames
  uint16_t demo_counter = demo_divider;


  // give sober mode a nice gentle fade
  uint8_t sober_fade_divider = 40 * FPS / 1000; // every 40ms = every 6 frames at 150FPS
  uint8_t sober_fade_counter = sober_fade_divider;
  uint8_t sober_mode_divider = FPS; // once per second
  uint8_t sober_mode_counter = sober_mode_divider;

  portb_val = 0;
  portb_mask = 0;

  uint8_t portb_mask_in = 0;
  uint8_t portb_val_in = 0;

  while(true) {

    one_frame_sample_handler();

    pushed = was_button_pressed();
    switch(pushed)
    {
      case SINGLE_CLICK:
        // toggle demo / sober
        render_mode ^= 1;
        mode = render_mode ? 13 : 0;
        break;

      case DOUBLE_CLICK:
      case LONG_PUSH:
        // exit
        return;

      case TRIPLE_CLICK:
        // for consistency with main, demo
        render_mode = 0;
        mode = 0;
        break;

      case QUADRUPLE_CLICK:
        // for consistency with main, sober
        render_mode = 1;
        mode = 13;
        break;

      case REALLY_LONG_PUSH:
        hard_reset(); // this never returns
        break;

      default:
        break;
    }

    if(render_mode == 1) {
      // sober
      fill_rainbow( leds, STRIP_LENGTH, 0, 7);
      portb_val_in = (seven_seg(13) << 4) | seven_seg(14);

      // send the 'leds' array out to the actual LED strip
      FastLED.show();

      if(--sober_mode_counter == 0) {
        sober_mode_counter = sober_mode_divider;
        mode = (mode <= 13) ? 14 : 13;
      }

      if(--sober_fade_counter == 0) {
        sober_fade_counter = sober_fade_divider;
        if(mode == 13) {
          switch (portb_mask_in) {
            default:
            case 0b00000000: portb_mask_in = 0b00000001; break;
            case 0b00000001: portb_mask_in = 0b00010001; break;
            case 0b00010001: portb_mask_in = 0b00010101; break;
            case 0b00010101: portb_mask_in = 0b01010101; break;
            case 0b01010101: portb_mask_in = 0b01010111; break;
            case 0b01010111: portb_mask_in = 0b01110111; break;
            case 0b01110111: portb_mask_in = 0b01111111; break;
            case 0b01111111: portb_mask_in = 0b11111111; break;
            case 0b11111111: portb_mask_in = 0b11111111; break;
          }
        } else {
          switch(portb_mask_in) {
            default:
            case 0b11111111: portb_mask_in = 0b11111110; break;
            case 0b11111110: portb_mask_in = 0b11101110; break;
            case 0b11101110: portb_mask_in = 0b10101110; break;
            case 0b10101110: portb_mask_in = 0b10101010; break;
            case 0b10101010: portb_mask_in = 0b00101010; break;
            case 0b00101010: portb_mask_in = 0b00100010; break;
            case 0b00100010: portb_mask_in = 0b00000010; break;
            case 0b00000010: portb_mask_in = 0b00000000; break;
            case 0b00000000: portb_mask_in = 0b00000000; break;
          }
        }
      }

    } else {

      // Call the current pattern function once, updating the 'leds' array
      gPatterns[gCurrentPatternNumber]();

      // send the 'leds' array out to the actual LED strip
      FastLED.show();

      // do some periodic updates

      /* 
        NOTE: the EVERY_N_MILLISECONDS etc macros from the original
        demo reel allocate static RAM, which is a waste when we're 
        only sometimes in this mode. We use local (stack based)
        counters instead
      */

      // slowly cycle the "base color" through the rainbow
      // EVERY_N_MILLISECONDS( 20 ) { gHue++; }
      if(--gHue_counter == 0) {
        gHue_counter = gHue_divider;
        gHue++; 
      }

      // EVERY_N_SECONDS( 10 ) { nextPattern(); }
      if(--demo_counter == 0) {
        demo_counter = demo_divider;
        nextPattern();  // change patterns periodically
      }

      // this gives a good framerate as is.
      switch (portb_mask_in) {
        case 0b00000000: portb_mask_in = 0b00000001; break;
        case 0b00000001: portb_mask_in = 0b00010001; break;
        case 0b00010001: portb_mask_in = 0b00010101; break;
        case 0b00010101: portb_mask_in = 0b01010101; break;
        case 0b01010101: portb_mask_in = 0b01010111; break;
        case 0b01010111: portb_mask_in = 0b01110111; break;
        case 0b01110111: portb_mask_in = 0b01111111; break;
        case 0b01111111: 
          mode++;
          mode = mode % 4;
          portb_mask_in = 0b11111111; 
          portb_val_in = (seven_seg(mode) << 4) | seven_seg(mode+4);
          break;
        case 0b11111111: portb_mask_in = 0b11111110; break;
        case 0b11111110: portb_mask_in = 0b11101110; break;
        case 0b11101110: portb_mask_in = 0b10101110; break;
        case 0b10101110: portb_mask_in = 0b10101010; break;
        case 0b10101010: portb_mask_in = 0b00101010; break;
        case 0b00101010: portb_mask_in = 0b00100010; break;
        case 0b00100010: portb_mask_in = 0b00000010; break;
        default: 
        case 0b00000010:
          portb_mask_in = 0b00000000;
          portb_val_in = (seven_seg((mode + 1) % 4) << 4) | seven_seg(mode+4);
          break;
      }
    }
    portb_mask = portb_mask_in;
    portb_val = portb_val_in;

    frame_epilogue();
  }
}

void demo_loop() {
  _demo_loop(0);
}

void sober_loop() {
  _demo_loop(1);
}

// void demo_loop() {

// //  uint8_t beat_sustain = 0;
//   byte is_beats = 0;
//   bool is_beat_1 = false;
//   bool is_beat_2 = false;
//   uint8_t vu_width = 0;
//   uint8_t mode = 1;


//   // stuff used to demonstrate the strip is working and FastLED with showN mod is working
//   // CRGB middle[1];

//   // middle[0].r = 0;
//   // middle[0].g = 255;
//   // middle[0].b = 0;

//   // uint8_t X = 0;

//   while(true) {

//     uint8_t pushed = false;
//     uint8_t min_vu = 0, max_vu = 255;

//     uint8_t new_sample_count_val = new_sample_count();
//     if(new_sample_count_val) {
//         // read these as they're volatile

//         pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
        
//         uint8_t sample_index = consume_sample_index();

//         vu_width = calculate_vu(sample_index, &min_vu, &max_vu, new_sample_count_val);

//         if(pushed == SINGLE_CLICK) {
// #ifdef VU_LOOKBEHIND
//           uint8_t lookbehind = VU_LOOKBEHIND;
// #else
//           uint8_t lookbehind = 20;
// #endif
//           uint8_t start = sample_index - lookbehind + 1;
//           uint8_t which_samples[lookbehind];

//           uint8_t i = 0;
//           do {
//             which_samples[i] = sampler.samples[(start + i) % SAMP_BUFF_LEN];
//             i++;
//           } while(i < lookbehind);
          
//           i = 0;
//           Serial.print(vu_width, HEX);
//           Serial.print(" @ ");
//           Serial.print(sample_index, HEX);
//           Serial.print(": ");
//           do {
//             Serial.print(" ");
//             Serial.print(which_samples[i], HEX);
//             i++;
//           } while(i < lookbehind);
//           Serial.println("");
//         }
    
//         uint8_t local_portb_val;
//         switch(mode) {
//           case 0: // show that beats are working
//             local_portb_val = 0; /* (is_beat_1 << 1) | (is_beat_2 << 2); */
//             portb_val = local_portb_val;
//             break;
//           case 1: // show that sampling is working
//             local_portb_val = 0;
//             if (vu_width > 128) local_portb_val |= 16;
//             if (vu_width > 64)  local_portb_val |= 8;
//             if (vu_width > 32)  local_portb_val |= 4;
//             if (vu_width > 16)  local_portb_val |= 2;
//             if (vu_width > 8)   local_portb_val |= 1;
//             portb_val = local_portb_val;
//             break;
//           case 2:
//             local_portb_val = 0;
//             portb_val = local_portb_val;
//             break;
//         }
    
//         debug_render_combo(false /* is_beat_2 */, false /* is_beat_1 */, sample_index);
    
//         #ifdef DEBUG_FRAME_RATE
//           DEBUG_FRAME_RATE_PORT |= (1 << DEBUG_FRAME_RATE_PIN);
//         #endif

//         FastLED.show();
//         // FastLED[0].show3(&leds[X], STRIP_LENGTH-X-1, &middle[0], 1, &leds[0], X, 255);

//         // static uint32_t last_time = 0;
//         // uint32_t this_time = micros();
//         // if (this_time > last_time + 100000) {
//         //   X++;
//         //   last_time = this_time;
//         // }
//         // if(X==STRIP_LENGTH-1) X=0;

//         #ifdef DEBUG_FRAME_RATE
//           DEBUG_FRAME_RATE_PORT &= ~(1 << DEBUG_FRAME_RATE_PIN);
//         #endif
        
//         reach_target_fps();
//     }
//   }  
// }
