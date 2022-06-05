/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _BUTTONS_H
#define _BUTTONS_H

#include <Arduino.h>
#include "config.h"
#include "framestate.h"

#define NO_PUSH 0
#define SINGLE_CLICK 1
#define DOUBLE_CLICK 2
#define TRIPLE_CLICK 3
#define QUADRUPLE_CLICK 4
#define QUINTUPLE_CLICK 5
#define SEXTUPLE_CLICK 6
#define LONG_PUSH 7

#ifndef BUTTON_CLICK_SPEED
#define BUTTON_CLICK_SPEED 300 // ms
#endif

#ifndef BUTTON_LONG_PUSH_SPEED
#define BUTTON_LONG_PUSH_SPEED 2000 // ms
#endif

uint8_t was_button_pressed(uint8_t pins);

#endif /* _BUTTONS_H */
