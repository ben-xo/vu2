/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "debug.h"

void setup_debug() {
  Serial.begin(2000000);
} 

void debug_loop() {

//  uint8_t beat_sustain = 0;
  byte is_beats = 0;
  bool is_beat_1 = false;
  bool is_beat_2 = false;
  uint8_t vu_width = 0;
  uint8_t mode = 1;


  // stuff used to demonstrate the strip is working and FastLED with showN mod is working
  // CRGB middle[1];

  // middle[0].r = 0;
  // middle[0].g = 255;
  // middle[0].b = 0;

  // uint8_t X = 0;

  while(true) {

    uint8_t pushed = false;
    uint8_t min_vu = 0, max_vu = 255;

    if(new_sample_count) {
        // read these as they're volatile

        pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
        
        cli();
        uint8_t sample_index = current_sample;
        new_sample_count--;
        sei();

        vu_width = calculate_vu(sample_index, &min_vu, &max_vu, new_sample_count);

        if(pushed == SHORT_PUSH) {
#ifdef VU_LOOKBEHIND
          uint8_t lookbehind = VU_LOOKBEHIND;
#else
          uint8_t lookbehind = 20;
#endif
          uint8_t start = sample_index - lookbehind + 1;
          uint8_t which_samples[lookbehind];

          uint8_t i = 0;
          do {
            which_samples[i] = sampler.samples[(start + i) % SAMP_BUFF_LEN];
            i++;
          } while(i < lookbehind);
          
          i = 0;
          Serial.print(vu_width, HEX);
          Serial.print(" @ ");
          Serial.print(sample_index, HEX);
          Serial.print(": ");
          do {
            Serial.print(" ");
            Serial.print(which_samples[i], HEX);
            i++;
          } while(i < lookbehind);
          Serial.println("");
        }
    
        uint8_t local_portb_val;
        switch(mode) {
          case 0: // show that beats are working
            local_portb_val = 0; /* (is_beat_1 << 1) | (is_beat_2 << 2); */
            portb_val = local_portb_val;
            break;
          case 1: // show that sampling is working
            local_portb_val = 0;
            if (vu_width > 128) local_portb_val |= 16;
            if (vu_width > 64)  local_portb_val |= 8;
            if (vu_width > 32)  local_portb_val |= 4;
            if (vu_width > 16)  local_portb_val |= 2;
            if (vu_width > 8)   local_portb_val |= 1;
            portb_val = local_portb_val;
            break;
          case 2:
            local_portb_val = 0;
            portb_val = local_portb_val;
            break;
        }
    
        debug_render_combo(false /* is_beat_2 */, false /* is_beat_1 */, sample_index);
    
        #ifdef DEBUG_FRAME_RATE
          DEBUG_FRAME_RATE_PORT |= (1 << DEBUG_FRAME_RATE_PIN);
        #endif

        FastLED.show();
        // FastLED[0].show3(&leds[X], STRIP_LENGTH-X-1, &middle[0], 1, &leds[0], X, 255);

        // static uint32_t last_time = 0;
        // uint32_t this_time = micros();
        // if (this_time > last_time + 100000) {
        //   X++;
        //   last_time = this_time;
        // }
        // if(X==STRIP_LENGTH-1) X=0;

        #ifdef DEBUG_FRAME_RATE
          DEBUG_FRAME_RATE_PORT &= ~(1 << DEBUG_FRAME_RATE_PIN);
        #endif
        
        reach_target_fps();
    }
  }  
}
