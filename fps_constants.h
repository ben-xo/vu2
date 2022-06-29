/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _FPS_CONSTANTS_H
#define _FPS_CONSTANTS_H

#include "config.h"

#define PWM_PRESCALER 8 // must match what enable_ledpwm() does

// e.g. at 16MHz, overflow val will be 200. At 20Mhz, 250. At 8MHz, 100.
#define PWM_OVERFLOW_VALUE (F_CPU / PWM_LED_FRQ / PWM_PRESCALER)

// e.g. at 16MHz and 10% duty, this will be 180. At 20Mhz, 225. At 8MHz, 90.
#define PWM_DUTY_VALUE     (PWM_OVERFLOW_VALUE - (PWM_OVERFLOW_VALUE / (100 / PWM_DUTY_PERCENT)))

#define FPS_INTERRUPT_RESET_VAL (PWM_LED_FRQ / FPS)

#endif /* _FPS_CONSTANTS_H */
