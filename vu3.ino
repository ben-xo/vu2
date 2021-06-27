/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "config.h"

// this define is for FastLED
#define NO_CORRECTION 1
#define FASTLED_ALLOW_INTERRUPTS 1
#define FASTLED_ACCURATE_CLOCK 1
#define DITHER 0
#include "framestate.h"

struct Framestate F; // the global instance

#include "ledpwm.h"
#include "sampler.h"
#include "tempo.h"

#ifdef BEAT_WITH_INTERRUPTS
// This mode doesn't currently work, because it's all been integrated into this project and isn't needed
// If you want to trigger the beats with interrupts, you'll need to reconfigure the BEAT_PIN_1 and 2 to inputs
// and take out the IIR filter etc down below.
#include "beatdetect.h"
volatile uint8_t beats_from_interrupt = 0;
#endif

#include "buttons.h"
#include "fps.h"
#include "debug.h"

#include <DigitalIO.h>

#include "debugrender.h"
#include "render.h"

CRGB leds[STRIP_LENGTH];

DigitalPin<BEAT_PIN_1> beat_pin;
DigitalPin<BEAT_PIN_2> tempo_pin;

void setup() {
  // put your setup code here, to run once:

  // the pin with the push button
  pinMode(BUTTON_PIN, INPUT);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  pinMode(DUTY_CYCLE_LED, OUTPUT);

  beat_pin.config(OUTPUT, LOW);
  tempo_pin.config(OUTPUT, LOW);
  
  // the pin with the push-button LED
  pinMode(BUTTON_LED_PIN,OUTPUT);  

  // the pin with the mode display
  pinMode(MODE_LED_PIN_1,OUTPUT);
  pinMode(MODE_LED_PIN_2,OUTPUT);
  pinMode(MODE_LED_PIN_3,OUTPUT);
  pinMode(MODE_LED_PIN_4,OUTPUT);
  pinMode(MODE_LED_PIN_5,OUTPUT);
  pinMode(BUTTON_LED_PIN,OUTPUT);
  
#ifdef DEBUG_FRAME_RATE
  // debugging pin for checking frame rate
  pinMode (DEBUG_FRAME_RATE_PIN, OUTPUT);
#endif

  FastLED.addLeds<NEOPIXEL, NEOPIXEL_PIN>(leds, 0, STRIP_LENGTH).setCorrection(TypicalLEDStrip);
  
  setup_render();
  setup_sampler(SAMPLER_TIMER_COUNTER_FOR(SAMP_FREQ));
  setup_tempo();
  setup_fps();
  setup_ledpwm();

#ifdef BEAT_WITH_INTERRUPTS
  setup_beatdetect();
#endif

  setup_initial_framestate();

  setup_debug();
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

#define SEG_G (1<<0)
#define SEG_F (1<<1)
#define SEG_A (1<<2)
#define SEG_B (1<<3)

uint8_t seven_seg(uint8_t mode) {
#ifdef SEVEN_SEG_MODE_DISPLAY
  switch(mode) {
    default:  return 0;
    case 0:  return (SEG_A);
    case 1:  return (SEG_B);
    case 2:  return (SEG_G);
    case 3:  return (SEG_F);
    case 4:  return (SEG_A | SEG_B);
    case 5:  return (SEG_B | SEG_G);
    case 6:  return (SEG_G | SEG_F);
    case 7:  return (SEG_F | SEG_A);
    case 8:  return (SEG_A | SEG_B | SEG_G);
    case 9:  return (SEG_B | SEG_G | SEG_F);
    case 10: return (SEG_G | SEG_F | SEG_A);
    case 11: return (SEG_F | SEG_A | SEG_B);
    case 12: return (SEG_A | SEG_B | SEG_G | SEG_F);
    case 13: return (SEG_A | SEG_G);
    case 14: return (SEG_F | SEG_B);

  }
#else
  return mode;
#endif
}

void loop() {
  // put your main code here, to run repeatedly:

  // hold down button at startup
  if(PIND & (1 << BUTTON_PIN)) {
    debug_loop();
  }
  
  F.is_beat_1 = false;
  F.is_beat_2 = false;
  F.vu_width = 0;
  F.mode = 0;
  F.last_mode = 0;
  F.auto_mode = true;
  F.is_silent = false;
  F.is_attract_mode = false;
  bool filter_beat = false;

  do_banner();

  portb_val = seven_seg(F.mode); // writes directly to pins 9-12

  while(true) {
    DEBUG_FRAME_RATE_LOW();
    DEBUG_SAMPLE_RATE_LOW();
        
    // read these as they're volatile
    uint8_t sample_ptr = current_sample;
    uint8_t pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
    
    if(pushed == SHORT_PUSH) {
      F.mode++;
      if(F.mode > MAX_MODE) F.mode = 0;
      portb_val = seven_seg(F.mode); // writes directly to pins 9-12
      F.auto_mode = false;
      F.is_attract_mode = false;
    } else if(pushed == LONG_PUSH) {
      F.auto_mode = true;
      F.mode = 0;
      portb_val = 0;
    }
    
#ifdef BEAT_WITH_INTERRUPTS
    // this won't be much use unless you also rip out the IIR code below…
    byte is_beats = beats_from_interrupt;
    F.is_beat_1 = is_beats & (1 << BEAT_PIN_1);
    F.is_beat_2 = is_beats & (1 << BEAT_PIN_2);
#endif

    F.min_vu = 0;
    F.max_vu = 255;

    // Currently, the lookbehind for the VU is always the number of samples queued up since the last VU (i.e. a whole Frame's worth)
    // With 5kHz sample rate and 125fps, this is usually 40 samples. But because the interrupts are staggered so they don't all fire at once,
    // occassionally it's 39 or 41.
#ifndef VU_LOOKBEHIND
    F.vu_width = calculate_vu(sample_ptr, &F.min_vu, &F.max_vu, new_sample_count);
#else
    F.vu_width = calculate_vu(sample_ptr, &F.min_vu, &F.max_vu, VU_LOOKBEHIND);
#endif

    uint8_t recent_max_vu = calculate_auto_gain_bonus(F.vu_width);
    F.vu_width = F.vu_width + scale8(F.vu_width, 255 - recent_max_vu);

    if (pushed || F.vu_width > ATTRACT_MODE_THRESHOLD) {
      // loudness: cancel attract mode, and so does a button press.
      F.is_silent = false;
      F.is_attract_mode = false;
    } else {
      // quiet: short or long?
      if(!F.is_silent) {
        // first loop of silence. Record time.
        silent_since = start_time; // note start time of silence
        F.is_silent = true;
      } else {
        // 2nd+ loop of silence. Long enough for attract mode?
        if (!F.is_attract_mode && ((start_time - silent_since)/1024 > ATTRACT_MODE_TIMEOUT)) {
          F.is_attract_mode = true;
        }
      }
    }

    DEBUG_FRAME_RATE_LOW();
    DEBUG_SAMPLE_RATE_HIGH();

    // now let's do some beat calculations

    // snapshot values.
    cli();
    uint8_t my_current_sample = current_sample;
    uint8_t my_new_sample_count = new_sample_count;
    sei();

    bool was_beat = filter_beat;

    bool is_beat_1 = false; // start calculation assuming no beat in this frame

    uint8_t my_sample_base = my_current_sample - new_sample_count;
    uint8_t offset = 0;
    do {
      uint8_t sample_idx = (my_sample_base + offset) % SAMP_BUFF_LEN;
      uint8_t val = samples[sample_idx];
      PeckettIIRFixedPoint(val, &filter_beat);
      set_beat_at(sample_idx, filter_beat);
      offset++;

      // If there was a beat edge detected at any point, set is_beat_1.
      // This gives a 1 frame resolution on beats, which is 8ms resolution at 125fps - good enough for us.
      // If we only checked the end of the frame, we might miss a beat that was very short.
      is_beat_1 |= filter_beat;
    } while(offset < my_new_sample_count);
    new_sample_count -= my_new_sample_count; // decrement the global new sample count

    DEBUG_SAMPLE_RATE_LOW();

    F.is_beat_1 = is_beat_1;
    if(!was_beat && F.is_beat_1) {
        record_rising_edge();
    }

    F.is_beat_2 = recalc_tempo(F.is_beat_2);

    DEBUG_FRAME_RATE_HIGH();
    DEBUG_SAMPLE_RATE_HIGH();

    if(F.is_attract_mode) {
      render_attract();
    } else {
      
      if(F.auto_mode && auto_mode_change(F.is_beat_1)) {
        F.last_mode = F.mode;
        while(F.mode == F.last_mode) F.mode = random8(MAX_MODE+1); // max is exclusive
        portb_val = seven_seg(F.mode); // writes directly to pins 9-12.
      }

      render(current_sample, sample_sum);
    }

    DEBUG_FRAME_RATE_LOW();
    DEBUG_SAMPLE_RATE_HIGH();

    FastLED.show();
    
    DEBUG_FRAME_RATE_HIGH();
    DEBUG_SAMPLE_RATE_LOW();
    reach_target_fps();
    F.frame_counter++;
  }
}
