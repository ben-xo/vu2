
#include "config.h"
//#include "filter.h"

#include "sampler.h"
#include "ultrafastneopixel.h"
#include "render.h"
#include "debugrender.h"

UltraFastNeoPixel strip = UltraFastNeoPixel(STRIP_LENGTH);

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
  pinMode(BUTTON_PIN, INPUT);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  pinMode(DUTY_CYCLE_LED, OUTPUT);
  
  // the pin with the push-button LED
  pinMode(BUTTON_LED_PIN,OUTPUT);  

  // the pin with the mode display
  pinMode(MODE_LED_PIN_1,OUTPUT);
  pinMode(MODE_LED_PIN_2,OUTPUT);
  pinMode(MODE_LED_PIN_3,OUTPUT);
  pinMode(MODE_LED_PIN_4,OUTPUT);

//  setup_filter();
  setup_render();
  setup_sampler();
//  Serial.begin(2000000);
//  randomSeed(analogRead(2));
}

static uint8_t calculate_vu(uint8_t sample_ptr, uint8_t sample_count) {
  static uint8_t max_val=0, min_val=255;
  static uint8_t vu_iterator;
  static uint8_t last_width;

  while(sample_count > 0) { 
    // Read ADC and center to +-128
    uint8_t int_sample = samples[sample_ptr];
    sample_count--;
    sample_ptr++;

    if(int_sample > max_val) max_val = int_sample;
    if(int_sample < min_val) min_val = int_sample;

    vu_iterator++;
    if(vu_iterator == 20) {
      last_width = max_val - min_val;
      vu_iterator = 0;
      max_val=0;
      min_val=255;
    }
    if(last_width > 0) last_width--;
  }

  return last_width;
}

static void reach_target_fps() {
  uint32_t end_time = micros();
  if (end_time < start_time) {
    start_time -= end_time;
  }
  uint32_t total_time = end_time - start_time;
  
  // frame target is 100fps, or 20k microseconds
  if(total_time > 5000) {
    slow=true;
  } else {
    delayMicroseconds(5000-total_time);
    slow=false;
  }
  start_time = end_time;
}

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

void loop() {
  // put your main code here, to run repeatedly:
  
  byte is_beats = 0;
  bool is_beat_1;
  bool is_beat_2;
  uint8_t vu_width = 0;
  uint8_t mode = 0;
  uint8_t last_mode = 0;
  bool auto_mode = true;
  bool is_silent = false;
  bool is_attract_mode = false;

  do_banner();

  start_time = micros();
  silent_since = start_time;
  while(true) {
    start_time = micros();
    
    // read these as they're volatile
    cli();
    uint8_t sample_ptr = current_sample;
    uint8_t sample_count = new_sample_count;
    new_sample_count = 0;
    sei();

    uint8_t pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
    if(pushed == SHORT_PUSH) {
      mode++;
      auto_mode = false;
      is_attract_mode = false;
      if(mode > 10) {
        mode = 0;
      }
      PORTB = (mode << 1); // writes directly to pins 9-12
    } else if(pushed == LONG_PUSH) {
      auto_mode = true;
      mode = 0;
      PORTB = (mode << 1); // writes directly to pins 9-12
    }
    
    is_beats = PIND & ((1 << BEAT_PIN_1) | (1 << BEAT_PIN_2)); // read once - port is volatile
    vu_width = calculate_vu(sample_ptr, sample_count);

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
        if (!is_attract_mode && ((start_time - silent_since)/1000 > ATTRACT_MODE_TIMEOUT)) {
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
        while(mode == last_mode) mode = random(0,11); // max is exclusive
        PORTB = (mode << 1); // writes directly to pins 9-12.
      }
      
      render(vu_width, is_beat_2, true, mode, 0, 0, is_beat_1, current_sample);
    }

    strip.show();

    reach_target_fps();
  }
}
