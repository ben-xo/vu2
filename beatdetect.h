/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// external beat detect using interrupts from another source.

extern volatile uint8_t beats_from_interrupt;
void setup_beatdetect();
