#include "config.h"

#include "ledpwm.h"
#include "sampler.h"
#include "beatdetect.h"

volatile uint8_t beats_from_interrupt = 0;

#define NO_CORRECTION 1
#include <FastLED.h>

#include "debugrender.h"

#include "render.h"

CRGB leds[STRIP_LENGTH];

uint32_t static start_time; // time each loop started.
uint32_t static silent_since; // time we've been silent since.
bool static slow = false; // track render time

#define SHORT_PUSH 1
#define LONG_PUSH 2

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

  FastLED.addLeds<NEOPIXEL, NEOPIXEL_PIN>(leds, STRIP_LENGTH).setCorrection(TypicalLEDStrip);
 
//  setup_filter();
  setup_render();
  setup_sampler();
  setup_ledpwm();
  setup_beatdetect();
//  Serial.begin(2000000);
//  randomSeed(analogRead(2));
}

static uint8_t calculate_vu(uint8_t sample_ptr) {
  // VU is always width of last 20 samples, wherever we happen to be right now.
  uint8_t max_val=0, min_val=255;
  for (uint8_t i = 0; i < VU_LOOKBEHIND; i++) {
    uint8_t int_sample = samples[(sample_ptr-i)%SAMP_BUFF_LEN];
    if(int_sample > max_val) max_val = int_sample;
    if(int_sample < min_val) min_val = int_sample;
  }
  return max_val - min_val;
}

#ifdef FRAME_RATE_LIMIT
static void reach_target_fps() {
  uint32_t end_time = micros();
  uint32_t total_time;
  if (end_time < start_time) {
    total_time = -end_time + start_time;
  } else {
    total_time = end_time - start_time;
  }
  
  if(total_time > FRAME_LENGTH_CYCLES) {
    slow=true;
  } else {
    slow=false;
    uint16_t delayCycles = ((FRAME_LENGTH_CYCLES-total_time) >> 2) - 8;
    while(delayCycles-- > 0);
  }
  start_time = end_time;
}
#endif

// returns true on the falling edge of a button push
static uint8_t was_button_pressed(uint8_t pins) {
  static bool is_down = false;
  static uint32_t last_push;
  if(is_down && !pins) {
    is_down = false;
    if(millis() - last_push > 3000) {
      // long push
      return LONG_PUSH;
    }
    // short push
    return SHORT_PUSH;
  }
  if(!is_down && pins) {
    is_down = true;
    last_push = millis();
  }
  // no push yet, although one may be in progress.
  return 0;
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

#ifdef DEBUG_ONLY
void loop() {
#else  
void debug_loop() {
#endif

//  uint8_t beat_sustain = 0;
  byte is_beats = 0;
  bool is_beat_1 = false;
  bool is_beat_2 = false;
  uint8_t vu_width = 0;
  uint8_t mode = 0;

  start_time = micros();
  silent_since = start_time;
  while(true) {
    start_time = micros();
    
    // read these as they're volatile
    cli();
    uint8_t sample_ptr = current_sample;
    sei();

    uint8_t pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
    if(pushed == SHORT_PUSH) {
      mode++;
      if(mode > MAX_MODE) {
        mode = 0;
      }
    }

    is_beats = beats_from_interrupt;
    is_beat_1 = is_beats & (1 << BEAT_PIN_1);
    is_beat_2 = is_beats & (1 << BEAT_PIN_2);

    // TODO: is beat sustain useful? I'm not sure.
//    if(is_beats && beat_sustain > 0) {
//      beat_sustain--;
//    }
//    if(beat_sustain == 0) {
//        is_beats = PIND & ((1 << BEAT_PIN_1) | (1 << BEAT_PIN_2)); // read once - port is volatile
//        if(is_beats) {
//          is_beat_1 = is_beats & (1 << BEAT_PIN_1);
//          is_beat_2 = is_beats & (1 << BEAT_PIN_2);
//  
//          beat_sustain = BEAT_SUSTAIN;
//        } else {
//          is_beat_1 = false;
//          is_beat_2 = false;
//        }
//    }

    vu_width = calculate_vu(sample_ptr);

    uint8_t local_portb_val;
    switch(mode) {
      case 0: // show that beats are working
        local_portb_val = (is_beat_1 << 1) | (is_beat_2 << 2);
        portb_val = local_portb_val;
        break;
      case 1: // show that sampling is working
        local_portb_val = 0;
        if (vu_width > 128) local_portb_val |= 32;
        if (vu_width > 64)  local_portb_val |= 16;
        if (vu_width > 32)  local_portb_val |= 8;
        if (vu_width > 16)  local_portb_val |= 4;
        if (vu_width > 8)   local_portb_val |= 2;
        portb_val = local_portb_val;
        break;
      case 2:
        local_portb_val = slow ? 62 : 0;
        portb_val = local_portb_val;
        break;
    }

    debug_render_combo(is_beat_2, is_beat_1, current_sample);

    FastLED.show();
#ifdef FRAME_RATE_LIMIT
    reach_target_fps();
#endif
  }  
}

#ifndef DEBUG_ONLY
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

  silent_since = micros();
  while(true) {
    start_time = micros();
    
    // read these as they're volatile
    uint8_t sample_ptr = current_sample;
    uint8_t pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
    
    if(pushed == SHORT_PUSH) {
      mode++;
      auto_mode = false;
      is_attract_mode = false;
      if(mode > MAX_MODE) mode = 0;
    } else if(pushed == LONG_PUSH) {
      auto_mode = true;
      mode = 0;
    }

    if(pushed) {
      portb_val = (mode << 1); // writes directly to pins 9-12
    }
    
    is_beats = beats_from_interrupt;
    is_beat_1 = is_beats & (1 << BEAT_PIN_1);
    is_beat_2 = is_beats & (1 << BEAT_PIN_2);

    // TODO: is beat sustain useful? I'm not sure.
//    if(is_beats && beat_sustain > 0) {
//      beat_sustain--;
//    }
//    if(beat_sustain == 0) {
//        is_beats = PIND & ((1 << BEAT_PIN_1) | (1 << BEAT_PIN_2)); // read once - port is volatile
//        if(is_beats) {
//          is_beat_1 = is_beats & (1 << BEAT_PIN_1);
//          is_beat_2 = is_beats & (1 << BEAT_PIN_2);
//  
//          beat_sustain = BEAT_SUSTAIN;
//        } else {
//          is_beat_1 = false;
//          is_beat_2 = false;
//        }
//    }
    vu_width = calculate_vu(sample_ptr);

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
        while(mode == last_mode) mode = random(0,MAX_MODE+1); // max is exclusive
        portb_val = (mode << 1); // writes directly to pins 9-12.
      }

      render(vu_width, is_beat_2, true, mode, is_beat_1, current_sample);
    }

    FastLED.show();

#ifdef FRAME_RATE_LIMIT
    reach_target_fps();
#endif
  }
}
#endif // DEBUG_ONLY