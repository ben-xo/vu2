/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include <Arduino.h>
#include "config.h"

extern uint32_t start_time;   // time each loop started.
extern uint32_t silent_since; // time we've been silent since.
extern bool slow;             // track render time

void reach_target_fps();
