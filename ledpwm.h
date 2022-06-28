/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// LED PWM using interrupts.
// Makes a huge assumption that you're using the entirety of PORTB for LEDs.
// Uses all three GPIOR registers for speed.
// GPIOR0 is used as a flags register 
// - bit 0 is used to trigger the sampler at half the PWM rate (e.g. 5kHz sampler for 10kHz PWM)
// - but 1 is used as an overflow flag for the fps_count() to tick the FPS
// GPIOR1 contains the PORTB value to write
// GPIOR2 contains the a rotating mask used to swap nibbles of GPIOR1 before writing

#ifndef _LEDPWM_H
#define _LEDPWM_H

#include <DigitalIO.h>

#include "render.h"
#include "framestate.h"

#include "gpio0.h"

extern uint8_t pwm_duty;

//extern uint8_t volatile portb_val;
#define ledpwm_status GPIOR0
#define portb_mask GPIOR1
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

extern DigitalPin<BEAT_PIN_1> beat_pin;
extern DigitalPin<BEAT_PIN_2> tempo_pin;


#endif /* _LEDPWM_H */
