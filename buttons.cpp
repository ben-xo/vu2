/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "buttons.h"

uint8_t was_button_pressed() {

  bool pins = button_pin.read();

  uint16_t now = (uint16_t) millis();

  if(!F.is_down && pins) {
    // pressed
    F.is_down = true;
    F.last_push = now;
    F.clicks++;
    return NO_PUSH;
  }

  if(!F.is_down && !pins) {
    // not pressed
    if(F.clicks) {
      // but was pressed recently
      uint8_t retval = NO_PUSH;
      if(now - F.last_push > BUTTON_LONG_PUSH_SPEED) { // default 2000ms
        retval = LONG_PUSH;
      } else if(now - F.last_push > BUTTON_CLICK_SPEED) { // default 300ms
        switch(F.clicks) {
          case 1:
            retval = SINGLE_CLICK;
            break;
          case 2:
            retval = DOUBLE_CLICK;
            break;
          case 3:
            retval = TRIPLE_CLICK;
            break;
          case 4:
            retval = QUADRUPLE_CLICK;
            break;
          case 5:
            retval = QUINTUPLE_CLICK;
            break;
          case 6:
          default:
            retval = SEXTUPLE_CLICK;
            break;
        }
      }
      if(retval) F.clicks = 0;
      return retval;
    }
  }

  if(F.is_down && !pins) {
    // released
    F.is_down = false;
  }

  // is_down && pins means "still down"
  // no push yet, although one may be in progress.
  return NO_PUSH;
}
