/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "buttons.h"

// // returns true on the falling edge of a button push
// uint8_t was_button_pressed(uint8_t pins) {
//   static bool is_down = false;
//   static uint16_t last_push;
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


uint8_t was_button_pressed(uint8_t pins) {
  static bool is_down = false;
  static uint8_t clicks = 0;
  static uint16_t last_down = 0;

  uint16_t now = (uint16_t) millis();

  if(!is_down && pins) {
    // pressed
    is_down = true;
    last_down = now;
    clicks++;
    return NO_PUSH;
  }

  if(!is_down && !pins) {
    // not pressed
    if(clicks) {
      // but was pressed recently
      if(now - last_down > 3000) {
        clicks = 0;
        return LONG_PUSH;
      } else if(now - last_down > 500) {
        clicks = 0;
        switch(clicks) {
          case 1:
            return SHORT_PUSH;
          case 2:
            return DOUBLE_CLICK;
          case 3:
          default:
            return TRIPLE_CLICK;
        }
      }
    }
    return NO_PUSH;
  }

  if(is_down && !pins) {
    // released
    is_down = false;
  }

  // is_down && pins means "still down"
  // no push yet, although one may be in progress.
  return NO_PUSH;
}
