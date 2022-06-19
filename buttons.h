/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _BUTTONS_H
#define _BUTTONS_H

#include <Arduino.h>
#include "config.h"
#include "ledpwm.h"
#include "sevenseg.h"
#include "framestate.h"

#include <DigitalIO.h>

#define NO_PUSH 0
#define SINGLE_CLICK 1
#define DOUBLE_CLICK 2
#define TRIPLE_CLICK 3
#define QUADRUPLE_CLICK 4
#define QUINTUPLE_CLICK 5
#define LONG_PUSH 6
#define REALLY_LONG_PUSH 7

#ifndef BUTTON_CLICK_SPEED
#define BUTTON_CLICK_SPEED 300 // ms
#endif

#ifndef BUTTON_LONG_PUSH_SPEED
#define BUTTON_LONG_PUSH_SPEED 2000 // ms
#endif

#ifndef BUTTON_REALLY_LONG_PUSH_SPEED
#define BUTTON_REALLY_LONG_PUSH_SPEED 4000 // ms
#endif

extern DigitalPin<BUTTON_PIN> button_pin;

uint8_t was_button_pressed();

#endif /* _BUTTONS_H */
