/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "buttons.h"


bool is_down = false;
uint8_t clicks = 0;
uint16_t last_push = 0;


// returns true on the falling edge of a button push
// uint8_t was_button_pressed(uint8_t pins) {
//   uint16_t now = (uint16_t) millis();

//   if(is_down && !pins) {
//     is_down = false;
//     if(now - last_push > 3000) {
//       // long push
//       return LONG_PUSH;
//     }
//     // short push
//     return SHORT_PUSH;
//   }
//   if(!is_down && pins) {
//     is_down = true;
//     last_push = now;
//   }
//   // no push yet, although one may be in progress.
//   return NO_PUSH;
// }

// void reset_button_state() {
//   is_down = false;
//   last_push = (uint16_t) millis();
// }

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
      if(now - last_push > 3000) {
        retval = LONG_PUSH;
      } else if(now - last_push > 500) {
        switch(clicks) {
          case 1:
            retval = SHORT_PUSH;
            break;
          case 2:
            retval = DOUBLE_CLICK;
            break;
          case 3:
          default:
            retval = TRIPLE_CLICK;
        }
      }
      clicks = 0;
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
