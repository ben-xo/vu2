/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _RENDER_H
#define _RENDER_H

#ifndef DEBUG_ONLY

#include "framestate.h"
#include "effects.h"

void setup_render();
void rainbowCycle(uint8_t);
void render(uint8_t sample_ptr, uint16_t sample_sum);
void do_banner();
void render_attract();

// and now: the actual effects

#include "fire.h"
#include "plume.h"

#endif

// modes 0 to MAX_MODE are effects
#define MAX_MODE 9

#endif /* _RENDER_H */
