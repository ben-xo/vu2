/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "config.h"

#include "ledpwm.h"
#include "sampler.h"
//#include "beatdetect.h"
#include "buttons.h"
#include "fps.h"
#include "debug_loop.h"

volatile uint8_t beats_from_interrupt = 0;

#define NO_CORRECTION 1
#include <FastLED.h>

#include "debugrender.h"

#include "render.h"

CRGB leds[STRIP_LENGTH];

void setup() {
  // put your setup code here, to run once:

  // the pin with the push button
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BEAT_PIN_1, INPUT);
  pinMode(BEAT_PIN_2, INPUT);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  pinMode(DUTY_CYCLE_LED, OUTPUT);
  
  // the pin with the push-button LED
  pinMode(BUTTON_LED_PIN,OUTPUT);  

  // the pin with the mode display
  pinMode(MODE_LED_PIN_1,OUTPUT);
  pinMode(MODE_LED_PIN_2,OUTPUT);
  pinMode(MODE_LED_PIN_3,OUTPUT);
  pinMode(MODE_LED_PIN_4,OUTPUT);
  
#ifdef DEBUG_FRAME_RATE
  // debugging pin for checking frame rate
  pinMode (DEBUG_FRAME_RATE_PIN, OUTPUT);
#endif

  FastLED.addLeds<NEOPIXEL, NEOPIXEL_PIN>(leds, STRIP_LENGTH).setCorrection(TypicalLEDStrip);
  FastLED.setDither( 0 );
  
//  setup_filter();
  setup_render();
  setup_sampler();
  setup_ledpwm();
//  setup_beatdetect();

  setup_debug();
//  randomSeed(analogRead(2));
}

// auto change every 8 bars
uint32_t static last_beat;
byte static beat_count = 0;
static bool auto_mode_change(bool is_beat) {
  if(!is_beat) return false;
  uint32_t now = millis();
  if(now - last_beat > AUTO_BEATS_SILENCE_THRESH) {
    // mode change anyway if this is the first beat in ages
    last_beat = now;
    beat_count = 0;
    return true;
  }
  if(now - last_beat > AUTO_BEATS_MIN_THRESH) {
    last_beat = now;
    beat_count++;
    if(beat_count >= AUTO_BEATS) {
      beat_count = 0;
      return true;
    }
  }
  return false;
}

void loop() {
  // put your main code here, to run repeatedly:

  // hold down button at startup
  if(PIND & (1 << BUTTON_PIN)) {
    debug_loop();
  }
  
//  uint8_t beat_sustain = 0;
  byte is_beats = 0;
  bool is_beat_1 = false;
  bool is_beat_2 = false;
  uint8_t vu_width = 0;
  uint8_t mode = 0;
  uint8_t last_mode = 0;
  bool auto_mode = true;
  bool is_silent = false;
  bool is_attract_mode = false;

  do_banner();

  while(true) {
    
    // read these as they're volatile
    int8_t sample_ptr = current_sample - 127; // signify
    uint8_t pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
    
    if(pushed == SHORT_PUSH) {
      mode++;
      if(mode > MAX_MODE) mode = 0;
      portb_val = (mode << 1); // writes directly to pins 9-12
      auto_mode = false;
      is_attract_mode = false;
    } else if(pushed == LONG_PUSH) {
      auto_mode = true;
      mode = 0;
      portb_val = 0;
    }
    
    is_beats = beats_from_interrupt;
    is_beat_1 = is_beats & (1 << BEAT_PIN_1);
    is_beat_2 = is_beats & (1 << BEAT_PIN_2);

    int8_t min_vu = -128, max_vu = 127;
    vu_width = calculate_vu(sample_ptr, &min_vu, &max_vu);

    if (pushed || vu_width > ATTRACT_MODE_THRESHOLD) {
      // loudness: cancel attract mode, and so does a button press.
      is_silent = false;
      is_attract_mode = false;
    } else {
      // quiet: short or long?
      if(!is_silent) {
        // first loop of silence. Record time.
        silent_since = start_time; // note start time of silence
        is_silent = true;
      } else {
        // 2nd+ loop of silence. Long enough for attract mode?
        if (!is_attract_mode && ((start_time - silent_since)/1024 > ATTRACT_MODE_TIMEOUT)) {
          is_attract_mode = true;
        }
      }
    }

    if(is_attract_mode) {
      render_attract();
    } else {
      
      is_beat_1 = is_beats & (1 << BEAT_PIN_1);
      is_beat_2 = is_beats & (1 << BEAT_PIN_2);
      if(auto_mode && auto_mode_change(is_beat_1)) {
        last_mode = mode;
        while(mode == last_mode) mode = random8(MAX_MODE+1); // max is exclusive
        portb_val = (mode << 1); // writes directly to pins 9-12.
      }

      render(vu_width, is_beat_2, mode, is_beat_1, current_sample, min_vu, max_vu);
    }

#ifdef DEBUG_FRAME_RATE
  DEBUG_FRAME_RATE_PORT |= (1 << DEBUG_FRAME_RATE_PIN);
#endif
    FastLED.show();
#ifdef DEBUG_FRAME_RATE
  DEBUG_FRAME_RATE_PORT &= ~(1 << DEBUG_FRAME_RATE_PIN);
#endif

    reach_target_fps();
  }
}
