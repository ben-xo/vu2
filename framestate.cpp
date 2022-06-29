/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */
 
#include "framestate.h"

struct Framestate F; // the global instance

void setup_initial_framestate() {
  F.auto_mode = true;
  F.is_silent = false;
  F.is_attract_mode = false;
  F.is_beat_1 = false;
  F.is_beat_2 = false;
  F.pushed = 0;
}
