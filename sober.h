/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _SOBER_H
#define _SOBER_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

#include "buttons.h"
#include "ledpwm.h"
#include "sevenseg.h"

extern CRGB leds[STRIP_LENGTH];
void sober_mode();

#endif /* _SOBER_H */
