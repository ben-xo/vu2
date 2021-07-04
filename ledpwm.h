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

//extern uint8_t volatile portb_val;
#define portb_val GPIOR2

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

#endif /* _LEDPWM_H */
