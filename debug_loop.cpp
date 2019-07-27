#include "debug_loop.h"

void setup_debug() {
#ifdef DEBUG_ONLY
  Serial.begin(2000000);
#endif
}

void debug_loop() {

//  uint8_t beat_sustain = 0;
  byte is_beats = 0;
  bool is_beat_1 = false;
  bool is_beat_2 = false;
  uint8_t vu_width = 0;
  uint8_t mode = 0;

  while(true) {
    
    // read these as they're volatile
    cli();
    uint8_t sample_ptr = current_sample;
    sei();

    uint8_t pushed = was_button_pressed(PIND & (1 << BUTTON_PIN));
    if(pushed == SHORT_PUSH) {
      mode++;
      if(mode > MAX_DEBUG_MODE) {
        mode = 0;
      }
    }

    is_beats = beats_from_interrupt;
    is_beat_1 = is_beats & (1 << BEAT_PIN_1);
    is_beat_2 = is_beats & (1 << BEAT_PIN_2);

    uint8_t min_vu = 0, max_vu = 255;
    vu_width = calculate_vu(sample_ptr, &min_vu, &max_vu);

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
