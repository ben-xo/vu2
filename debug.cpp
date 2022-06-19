/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 *
 * Most of this file is adapted from FastLED DemoReel100.ino by Mark Kriegsman.
 */

#include "debug.h"
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


void setup_debug() {
  Serial.begin(2000000);
} 

  
void debug_loop()
{
  uint8_t pushed = NO_PUSH;
  uint8_t mode = 4;

  while(true) {

    one_frame_sample_handler();

    pushed = was_button_pressed();
    if(pushed)
    {
      return;
    }

    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();  
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FPS); 

    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

    EVERY_N_MILLISECONDS( 100 ) {
      mode++;
      if(mode > 7) {
        mode = 4;
      }
      portb_val = seven_seg(mode);
    }
  }
}


// void debug_loop() {

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
