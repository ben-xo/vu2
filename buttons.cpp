/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "buttons.h"

// returns true on the falling edge of a button push
uint8_t was_button_pressed(uint8_t pins) {
  static bool is_down = false;
  static uint16_t last_push;
  uint16_t now = (uint16_t) millis();

  if(is_down && !pins) {
    is_down = false;
    if(now - last_push > 3000) {
      // long push
      return LONG_PUSH;
    }
    // short push
    return SHORT_PUSH;
  }
  if(!is_down && pins) {
    is_down = true;
    last_push = now;
  }
  // no push yet, although one may be in progress.
  return 0;
}
