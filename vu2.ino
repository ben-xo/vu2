
#include "config.h"
#include "render.h"
#include "filter.h"
#include "sampler.h"
#include "ultrafastneopixel.h"

// debugging renderers.
#include "debugrender.c"

unsigned long time;

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

  time = micros(); // Used to track rate

  setup_filter();
//  setup_render();
  setup_sampler();
}

void loop() {
  // put your main code here, to run repeatedly:
  UltraFastNeoPixel the_strip = UltraFastNeoPixel(STRIP_LENGTH);
  the_strip.begin();
  the_strip.clear();
  the_strip.show();

  float envelope, value, float_sample, beat;
  bool is_beat = false;
  uint8_t beat_detect_iterator = 200;

  while(true) {
    
    // read these as they're volatile
    cli();
    uint8_t sample_ptr = current_sample;
    uint8_t sample_count = new_sample_count;
    new_sample_count = 0;
    sei();

    while(sample_count > 0) { 
      // Read ADC and center to +-128
      uint16_t int_sample = samples[sample_ptr & ((SAMP_BUFF_LEN * 8) - 1)] * 2;
      if(int_sample < 2) int_sample = 0;
      sample_count--;
      sample_ptr++;
      
      float_sample = (float)int_sample - 250.f;
      
      // Filter only bass component
      value = bassFilter(float_sample);

      // Take signal amplitude and filter
      if(value < 0)value=-value;
      envelope = envelopeFilter(value);

      beat_detect_iterator--;
      if(beat_detect_iterator == 0) {
        // do this only once every 200 samples (25Hz)
        beat = beatFilter(envelope);
        is_beat = beat > (0.08f * DEFAULT_THRESHOLD);
        beat_detect_iterator = 200;
      }
    }

//    debug_render_is_beat(the_strip, is_beat);
//    debug_render_samples(the_strip, sample_ptr, true);
    debug_render_combo(the_strip, is_beat, sample_ptr);
     __builtin_avr_delay_cycles( 100 ); 
  }
}
