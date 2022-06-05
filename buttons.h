/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _BUTTONS_H
#define _BUTTONS_H

#include <Arduino.h>

#define NO_PUSH 0
#define SINGLE_CLICK 1
#define DOUBLE_CLICK 2
#define TRIPLE_CLICK 3
#define QUADRUPLE_CLICK 4
#define QUINTUPLE_CLICK 5
#define SEXTUPLE_CLICK 6
#define LONG_PUSH 7

uint8_t was_button_pressed(uint8_t pins);

#endif /* _BUTTONS_H */
