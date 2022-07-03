/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// this define is for FastLED
#define NO_CLOCK_CORRECTION 1
#define FASTLED_ALLOW_INTERRUPTS 1
#define NO_MINIMUM_WAIT 1
// #define TRINKET_SCALE 0


#include "config.h"

#include "sampler.h"
#include "ledpwm.h"
#include "tempo.h"

#include "sevenseg.h"
#include "framestate.h"

#include "loop.h"
bool filter_beat = false;
uint8_t my_current_sample = 0;
uint16_t my_sample_sum = 0;

#ifdef BEAT_WITH_INTERRUPTS
// This mode doesn't currently work, because it's all been integrated into this project and isn't needed
// If you want to trigger the beats with interrupts, you'll need to reconfigure the BEAT_PIN_1 and 2 to inputs
// and take out the IIR filter etc down below.
#include "beatdetect.h"
volatile uint8_t beats_from_interrupt = 0;
#endif

#include "buttons.h"
#include "hardreset.h"
#include "fps.h"
#include "demo.h"

#include <DigitalIO.h>

#include "debugrender.h"
#include "render.h"

CRGB leds[STRIP_LENGTH];

void setup() {
  // put your setup code here, to run once:

  // the pin with the push button
  pinMode(NEOPIXEL_PIN, OUTPUT);

  beat_pin.config(OUTPUT, LOW);
  tempo_pin.config(OUTPUT, LOW);
  button_pin.config(INPUT, LOW);
  
  // the pin with the push-button LED
  pinMode(BUTTON_LED_PIN,OUTPUT);  

  // the pin with the mode display
  pinMode(MODE_LED_PIN_1,OUTPUT);
  pinMode(MODE_LED_PIN_2,OUTPUT);
  pinMode(MODE_LED_PIN_3,OUTPUT);
  pinMode(MODE_LED_PIN_4,OUTPUT);
  pinMode(MODE_LED_PIN_5,OUTPUT);
  pinMode(BUTTON_LED_PIN,OUTPUT);
  
  FastLED.addLeds<NEOPIXEL, NEOPIXEL_PIN>(leds, STRIP_LENGTH).setCorrection(TypicalLEDStrip);

  FastLED.setDither( 0 );
  
  setup_render();
  setup_sampler(SAMPLER_TIMER_COUNTER_FOR(SAMP_FREQ));
  setup_tempo();
  setup_fps();
  setup_ledpwm();
  setup_debug();

#ifdef BEAT_WITH_INTERRUPTS
  setup_beatdetect();
#endif

  setup_initial_framestate();

  setup_demo();

}

// auto change every 8 bars
static bool auto_mode_change(bool is_beat) {
  uint16_t static last_beat;
  byte static beat_count = 0;
  if(!is_beat) return false;
  uint16_t now = (uint16_t)millis();
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

static void ledpwm_reset()
{
    set_status_leds(seven_seg(F.mode));
}

/*
 * Mess with the brightness of the status LEDs (by adjusting the mask and double-buffer content) 
 * so that it's VU reactive. Kinda ugly but also kinda cool at the same time.
 */
static void ledpwm_vu_1() {

    static const PROGMEM uint8_t masks[16] = { 
      0b01111111, 0b01110111, 0b01110101, 0b01010101,
      0b01010100, 0b01000100, 0b01000000, 0b00000000,
      0b01010101, 0b01010101, 0b01010101, 0b01010101,
      0b01010101, 0b01010101, 0b01010101, 0b01010101
    };

    // there are only 4 lights, so 5 brightness levels
    static const PROGMEM uint8_t vals[8] = { 
      0b00000000, 0b00000000, 0b00000000, 0b00000000,
      0b00010000, 0b01010000, 0b01110000, 0x11110000
    };
    
    uint8_t four_bit_level = (F.vu_width >> 4) & 0x0F;
    uint8_t new_portb_mask = masks[four_bit_level];

    int8_t three_bit_level = (four_bit_level >> 1) & 0x07;
    uint8_t new_portb_val  = vals[three_bit_level] | seven_seg(F.mode);

    // if(four_bit_level > 0x0B) {
    //     new_portb_val = 0b11110000;
    // } else if(four_bit_level > 0x07) {
    //     if(F.frame_counter & 0x01) {
    //       new_portb_val = 0b10100000;
    //     } else {
    //       new_portb_val = 0b01010000;
    //     }
    // }
    // new_portb_val |= seven_seg(F.mode);

    set_status_leds_and_mask(new_portb_val, new_portb_mask);
}

void loop() {
  // put your main code here, to run repeatedly:

  // hold down button at startup
  if(button_pin.read()) {
    demo_loop();
  }
  
  F.is_beat_1 = false;
  F.is_beat_2 = false;
  F.vu_width = 0;
  F.mode = 0;
  F.last_mode = 0;
  F.auto_mode = true;
  F.is_silent = false;
  F.is_attract_mode = false;
  F.pushed = false;

#if DO_BANNER
  do_banner();
#endif

  ledpwm_reset();

  while(true) {

    one_frame_sample_handler();

    if(F.is_attract_mode) {
      render_attract();
    } else {
      
      if(F.auto_mode && auto_mode_change(F.is_beat_1)) {
        F.last_mode = F.mode;
        while(F.mode == F.last_mode) F.mode = random8(MAX_MODE+1); // max is exclusive
        // the mode indicator LED is changed by ledpwm_vu_1() below
      }

      render(my_current_sample, my_sample_sum);
    }

    DEBUG_SAMPLE_RATE_HIGH();

    FastLED.show();
    //FastLED[0].show(&leds[0], STRIP_LENGTH, 255);

    ledpwm_vu_1();

    // do post-frame-render stuff
    uint8_t pushed = was_button_pressed();
    F.pushed = (bool)pushed;
    switch(pushed)
    {
      case SINGLE_CLICK:
        // change mode (cancels auto change)
        F.auto_mode = false;
        F.is_attract_mode = false;
        F.mode++;
        if(F.mode > MAX_MODE) F.mode = 0;
        ledpwm_reset();
        break;

      case LONG_PUSH:
      case DOUBLE_CLICK:
        // reinstate auto change
        F.auto_mode = true;
        F.is_attract_mode = false;
        F.mode = 0;
        ledpwm_reset();
        break;

      case TRIPLE_CLICK:
        demo_loop();
        ledpwm_reset();
        break;

      case QUADRUPLE_CLICK:
        sober_loop();
        ledpwm_reset();
        break;

      case REALLY_LONG_PUSH:
        hard_reset(); // this never returns
        break;

      default:
        break;
    }



    frame_epilogue();
  }
}
