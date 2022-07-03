/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _PWM_CONSTANTS_H
#define _PWM_CONSTANTS_H

#include <Arduino.h>
#include "config.h"

// 0b11111111 is "invisible" and 0b00000000 is "super bright" not that the human eye can tell
#ifndef MASK_RESET_VAL
#  define MASK_RESET_VAL 0b00000000
#endif

// These values are specific to the LEDs you choose. 
// Override them in config.h if you like.

#ifndef PWM_LED_FRQ
#  define PWM_LED_FRQ 10000 // 10kHz
#endif

#ifndef PWM_DUTY_PERCENT
#  define PWM_DUTY_PERCENT 10
#endif

#define PWM_PRESCALER 8 // must match what enable_ledpwm() does

// e.g. at 16MHz, overflow val will be 200. At 20Mhz, 250. At 8MHz, 100.
#define PWM_OVERFLOW_VALUE (F_CPU / PWM_LED_FRQ / PWM_PRESCALER)

// e.g. at 16MHz and 10% duty, this will be 180. At 20Mhz, 225. At 8MHz, 90.
#define PWM_DUTY_VALUE     (PWM_OVERFLOW_VALUE - (PWM_OVERFLOW_VALUE / (100 / PWM_DUTY_PERCENT)))

// starts the timer at an offset so that PWM interrupts don't coincide with other interrupts.
// offset should not be between PWM_DUTY_VALUE and PWM_OVERFLOW_VALUE, and preferably quite far from them
#define PWM_STARTING_OFFSET (PWM_DUTY_VALUE / 2)

/* definition to expand macro then apply to pragma message */
/* from https://stackoverflow.com/questions/1562074/how-do-i-show-the-value-of-a-define-at-compile-time */
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

// compile time debug to see the PWM vals
#pragma message(VAR_NAME_VALUE(PWM_OVERFLOW_VALUE))
#pragma message(VAR_NAME_VALUE(PWM_DUTY_VALUE))

#endif /* _PWM_CONSTANTS_H */
