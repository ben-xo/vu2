/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */


#ifndef _SEVENSEG_H
#define _SEVENSEG_H

#include <Arduino.h>
#include "config.h"

#define SEG_G (1<<0)
#define SEG_F (1<<1)
#define SEG_A (1<<2)
#define SEG_B (1<<3)

uint8_t seven_seg(uint8_t mode);

#endif /* _SEVENSEG_H */
