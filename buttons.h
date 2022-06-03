/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _BUTTONS_H
#define _BUTTONS_H

#include <Arduino.h>

#define NO_PUSH 0
#define SHORT_PUSH 1
#define LONG_PUSH 2
#define DOUBLE_CLICK 3
#define TRIPLE_CLICK 4

uint8_t was_button_pressed(uint8_t pins);

#endif /* _BUTTONS_H */
