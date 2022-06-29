/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _DEMO_H
#define _DEMO_H

#include <Arduino.h>

#include "config.h"

#include "ledpwm.h"
#include "sampler.h"
#include "beatdetect.h"
#include "buttons.h"
#include "fps.h"

#include "sevenseg.h"

#include "debugrender.h"

extern uint32_t start_time;
extern uint32_t silent_since;

void setup_demo();
void demo_loop();
void sober_loop();

#endif /* _DEMO_H */
