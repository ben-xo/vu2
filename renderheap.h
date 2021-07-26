/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _RENDERHEAP_H
#define _RENDERHEAP_H

#include <Arduino.h>
#include "config.h"
#include <FastLED.h>

typedef struct {
  uint8_t hue, beat_offset;
} render_vu_with_beat_strobe_type;

typedef struct {
  uint8_t beat_brightness;
} render_vu_plus_beat_interleave_type;

typedef struct {
  uint8_t phase;
} render_beat_line_type;

typedef struct {
  bool was_beat_2;
  uint8_t fade_type;
} render_double_vu_type;

typedef struct {
  bool was_beat;
  bool top;
  uint16_t hue;
} render_beat_bounce_flip_type;

typedef struct {
  uint32_t dot_pos[ATTRACT_MODE_DOTS];
  uint16_t dot_speeds[ATTRACT_MODE_DOTS];
  CRGB dot_colors[ATTRACT_MODE_DOTS];
  uint8_t dot_age[ATTRACT_MODE_DOTS];
} attract_mode_type;

typedef struct {
  byte heat[STRIP_LENGTH];
} fire_mode_type;

typedef union renderheap_t {
  render_vu_with_beat_strobe_type     rvuwbs;
  render_vu_plus_beat_interleave_type rvupbi;
  render_beat_line_type               rbl;
  render_double_vu_type               rdv;
  render_beat_bounce_flip_type        rbbf;
  attract_mode_type                   am;
  fire_mode_type                      fm;
};

extern renderheap_t r;

#endif /* _EFFECTS_H */
