/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#include "sober.h"

void sober_mode() {

  uint8_t pushed = NO_PUSH;
  uint8_t mode = 13;

  while(true) {

    pushed = was_button_pressed();
    if(pushed)
    {
      return;
    }

    fill_rainbow( leds, STRIP_LENGTH, 0, 7);

    // send the 'leds' array out to the actual LED strip
    FastLED.show();  
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FPS); 

    EVERY_N_SECONDS( 1 ) {
      portb_val = seven_seg(mode);
      mode = (mode == 13) ? 14 : 13;
    }
  }  
}
