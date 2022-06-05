/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "buttons.h"


bool is_down = false;
uint8_t clicks = 0;
uint16_t last_push = 0;

uint8_t was_button_pressed(uint8_t pins) {

  uint16_t now = (uint16_t) millis();

  if(!is_down && pins) {
    // pressed
    is_down = true;
    last_push = now;
    clicks++;
    return NO_PUSH;
  }

  if(!is_down && !pins) {
    // not pressed
    if(clicks) {
      // but was pressed recently
      uint8_t retval = NO_PUSH;
      if(now - last_push > 2000) {
        retval = LONG_PUSH;
      } else if(now - last_push > 300) {
        switch(clicks) {
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
      if(retval) clicks = 0;
      return retval;
    }
  }

  if(is_down && !pins) {
    // released
    is_down = false;
  }

  // is_down && pins means "still down"
  // no push yet, although one may be in progress.
  return NO_PUSH;
}
