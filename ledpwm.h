/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// LED PWM using interrupts.
// Makes a huge assumption that you're using the entirety of PORTB for LEDs.

#ifndef _LEDPWM_H
#define _LEDPWM_H

#include <DigitalIO.h>

#include "render.h"
#include "framestate.h"

extern uint8_t pwm_duty;
extern uint8_t volatile portb_val;
void setup_ledpwm();
void disable_ledpwm();
void enable_ledpwm();

// These values are specific to the LEDs you choose. 
// Override them in config.h if you like.

#ifndef PWM_LED_FRQ
#  define PWM_LED_FRQ 10000 // 10kHz
#endif

#ifndef PWM_DUTY_PERCENT
#  define PWM_DUTY_PERCENT 10
#endif

// must match what enable_ledpwm() does.
#define PWM_PRESCALER 1

// e.g. at 16MHz, overflow val will be 200. At 20Mhz, 250. At 8MHz, 100.
#define PWM_OVERFLOW_VALUE (F_CPU / PWM_LED_FRQ / PWM_PRESCALER)

// e.g. at 16MHz, duty val will be 180. At 20Mhz, 225. At 8MHz, 90.
#define PWM_DUTY_VALUE     (PWM_OVERFLOW_VALUE - (PWM_OVERFLOW_VALUE / (100 / PWM_DUTY_PERCENT)))

#endif /* _LEDPWM_H */
