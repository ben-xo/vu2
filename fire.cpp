/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

//void loop() {
//  Fire(55,120,15);
//}

#include "config.h"
#include "render.h"

#define COOLING 65
#define SPARKING 120
#define COOLING_RANDOM_FACTOR (((COOLING * 10) / STRIP_LENGTH) + 2)

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = scale8(temperature, 191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    leds[Pixel] = CRGB(255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    leds[Pixel] = CRGB(255, heatramp, 0);
  } else {                               // coolest
    leds[Pixel] = CRGB(heatramp, 0, 0);
  }
}

void render_fire(bool is_beat, unsigned int peakToPeak) {
  static byte heat[STRIP_LENGTH]; // todo: this is probably wasteful
  int cooldown;
  
  // Step 1.  Cool down every cell a little
  for( uint8_t i = 0; i < STRIP_LENGTH; i++) {
    cooldown = random8(COOLING_RANDOM_FACTOR);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( uint8_t k= STRIP_LENGTH - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random8() < peakToPeak ) {
    uint8_t y = random8(7);
    heat[y] = random8(95) + 160;
    //heat[y] = random(160,255);
  }

  if(is_beat) {
    // always spark on the beat
    uint8_t y = random8(STRIP_LENGTH/3);
    heat[y] = random8(95) + 160;
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < STRIP_LENGTH; j++) {
    setPixelHeatColor(j, heat[j] );
  }
}
