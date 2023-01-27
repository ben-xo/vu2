/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _BEATDETECT_H
#define _BEATDETECT_H

#include "config.h"

// external beat detect using interrupts from another source.

#ifdef BEAT_WITH_INTERRUPTS
extern volatile uint8_t beats_from_interrupt;
void setup_beatdetect();
#endif


#endif /* _BEATDETECT_H */