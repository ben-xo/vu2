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

static uint8_t seven_seg(uint8_t mode)
{
#ifdef SEVEN_SEG_MODE_DISPLAY
  switch(mode) {
    default:  return 0;
    case 0:  return (SEG_A);
    case 1:  return (SEG_B);
    case 2:  return (SEG_G);
    case 3:  return (SEG_F);
    case 4:  return (SEG_A | SEG_B);
    case 5:  return (SEG_B | SEG_G);
    case 6:  return (SEG_G | SEG_F);
    case 7:  return (SEG_F | SEG_A);
    case 8:  return (SEG_A | SEG_B | SEG_G);
    case 9:  return (SEG_B | SEG_G | SEG_F);
    case 10: return (SEG_G | SEG_F | SEG_A);
    case 11: return (SEG_F | SEG_A | SEG_B);
    case 12: return (SEG_A | SEG_B | SEG_G | SEG_F);
    case 13: return (SEG_A | SEG_G);
    case 14: return (SEG_F | SEG_B);

  }
#else
  return mode;
#endif
}

#endif /* _SEVENSEG_H */
