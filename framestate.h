/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _FRAMESTATE_H
#define _FRAMESTATE_H

#include <Arduino.h>

struct Framestate
{
  uint16_t frame_counter = 0;
  uint8_t vu_width = 0;
  uint8_t mode = 0;
  uint8_t last_mode = 0;
  uint8_t min_vu = 0;
  uint8_t max_vu = 255;
  bool auto_mode:1;
  bool is_silent:1;
  bool is_attract_mode:1;
  bool is_beat_1:1;
  bool is_beat_2:1;
};

extern Framestate F;

void setup_initial_framestate();
#endif /* _FRAMESTATE_H */
