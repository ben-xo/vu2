
#include "config.h"
//#include "filter.h"

#include "sampler.h"
#include "ultrafastneopixel.h"
#include "render.h"
#include "debugrender.h"

UltraFastNeoPixel strip = UltraFastNeoPixel(STRIP_LENGTH);

uint32_t start_time;
bool slow = false; // track render time

void setup() {
  // put your setup code here, to run once:

  // the pin with the push button
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BEAT_PIN_1, INPUT);
  pinMode(BEAT_PIN_2, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  
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
}

uint8_t calculate_vu(uint8_t sample_ptr, uint8_t sample_count) {
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

void reach_target_fps() {
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

// returns true on the rising edge of a button push
bool was_button_pressed(uint8_t pins) {
  static bool is_down = false;
  if(is_down && !pins) {
    is_down = false;
    return true;
  }
  if(!is_down && pins) {
    is_down = true;
  }
  return false;
}

void loop() {
  // put your main code here, to run repeatedly:
  
  bool is_beat_1 = false;
  bool is_beat_2 = false;
  uint8_t vu_width = 0;
  uint8_t mode = 0;

  start_time = micros();
  while(true) {
    
    // read these as they're volatile
    cli();
    uint8_t sample_ptr = current_sample;
    uint8_t sample_count = new_sample_count;
    new_sample_count = 0;
    sei();

    vu_width = calculate_vu(sample_ptr, sample_count);
    is_beat_1 = PIND & (1 << BEAT_PIN_1);
    is_beat_2 = PIND & (1 << BEAT_PIN_2);

    if(was_button_pressed(PIND & (1 << BUTTON_PIN))) {
      mode++;
      if(mode > 10) {
        mode = 0;
      }
    }
    render(vu_width, is_beat_2, true, mode, 0, 0, is_beat_1, current_sample);

    strip.show();

    reach_target_fps();
  }
}
