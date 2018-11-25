
#include "config.h"
#include "render.h"
#include "filter.h"
#include "sampler.h"
#include "ultrafastneopixel.h"
#include "debugrender.h"

uint32_t start_time;
bool slow = false; // track render time

void setup() {
  // put your setup code here, to run once:

  // the pin with the push button
  pinMode(BUTTON_PIN, INPUT);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  
  // the pin with the push-button LED
  pinMode(BUTTON_LED_PIN,OUTPUT);  

  // the pin with the mode display
  pinMode(MODE_LED_PIN_1,OUTPUT);
  pinMode(MODE_LED_PIN_2,OUTPUT);
  pinMode(MODE_LED_PIN_3,OUTPUT);
  pinMode(MODE_LED_PIN_4,OUTPUT);

  setup_filter();
//  setup_render();
  setup_sampler();
}

bool calculate_beat_detect(uint8_t sample_ptr, uint8_t sample_count) {
  static float envelope, value, float_sample, beat;
  static bool is_beat;
  static uint8_t beat_detect_iterator;

  while(sample_count > 0) { 
    // Read ADC and center to +-128
    uint16_t int_sample = samples[sample_ptr & ((SAMP_BUFF_LEN * 8) - 1)] * 2;
    if(int_sample < 2) int_sample = 0;
    sample_count--;
    sample_ptr++;
    
    float_sample = (float)int_sample - 250.f;
    
    // Filter only bass component
//    value = bassFilter(float_sample);
    value=float_sample;

    // Take signal amplitude and filter
    if(value < 0)value=-value;
    envelope = envelopeFilter(value);
//    envelope=value;

    beat_detect_iterator++;
    if(beat_detect_iterator == 203) {
      // do this only once every 200 samples (25Hz)
      beat = beatFilter(envelope);
      is_beat = beat > (0.08f * DEFAULT_THRESHOLD);
      beat_detect_iterator = 3;
    }
  }

  return is_beat;
}

uint8_t calculate_vu(uint8_t sample_ptr, uint8_t sample_count) {
  static uint8_t max_val;
  static uint8_t vu_iterator;
  static uint8_t last_width;

  while(sample_count > 0) { 
    // Read ADC and center to +-128
    uint8_t int_sample = samples[sample_ptr & ((SAMP_BUFF_LEN * 8) - 1)];
    if(int_sample < 2) int_sample = 0;
    sample_count--;
    sample_ptr++;

    max_val = max(int_sample, max_val);

    vu_iterator++;
    if(vu_iterator == 35) { // range: 30 to 5, to offset from beat detect
      last_width = max_val;
      vu_iterator = 10;
      max_val=0;
    }
  }

  return last_width;
}

void reach_target_fps() {
  uint32_t end_time = micros();
  uint32_t total_time = end_time - start_time;
  if (end_time < start_time) {
    total_time -= start_time;
  }
  // frame target is 100fps, or 20k microseconds
  if(total_time > 10000) {
    slow=true;
  } else {
    delayMicroseconds(10000-total_time);
    slow=false;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  UltraFastNeoPixel the_strip = UltraFastNeoPixel(STRIP_LENGTH);
  the_strip.begin();
  the_strip.clear();
  the_strip.show();

  bool is_beat = false;
  uint8_t vu_width = 0;

  while(true) {

    start_time = micros();
    
    // read these as they're volatile
    cli();
    uint8_t sample_ptr = current_sample;
    uint8_t sample_count = new_sample_count;
    new_sample_count = 0;
    sei();

// TODO: make these fast enough!
    is_beat = calculate_beat_detect(sample_ptr, sample_count);
    vu_width = calculate_vu(sample_ptr, sample_count);

//    debug_render_vu(the_strip, vu_width);
    debug_render_is_beat(the_strip, is_beat);
//    debug_render_combo(the_strip, is_beat, sample_ptr);
    if(slow) {
      the_strip.setPixelColor(0, 255,0,0);
    } else {
      the_strip.setPixelColor(0, 0,255,0);
    }
//    for (int i = 0; i < sample_count; i++) {
//      the_strip.setPixelColor(i+1, 0,0,255);
//    }
//    for (int i = sample_count; i < STRIP_LENGTH; i++) {
//      the_strip.setPixelColor(i+1, 0,0,0);
//    }
//    debug_render_samples(the_strip, sample_ptr, true);
    the_strip.show();

    reach_target_fps();
  }
}
