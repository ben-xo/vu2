/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// LED PWM using interrupts.

// Makes a huge assumption that you're using the entirety of PORTB for LEDs.

// Uses all three GPIOR registers for speed.

// GPIOR0 is used as a flags register. See gpio0.h for bit meanings
// GPIOR1 contains the PORTB value to write
// GPIOR2 contains the a rotating mask used to swap nibbles of GPIOR1 before writing

#ifndef _LEDPWM_H
#define _LEDPWM_H

#include <Arduino.h>
#include <DigitalIO.h>

#include "render.h"
#include "framestate.h"
#include "pwm_constants.h"
#include "gpio0.h"

#define ledpwm_status GPIOR0
#define portb_mask GPIOR1
#define portb_val GPIOR2

void setup_ledpwm();
void disable_ledpwm();
void enable_ledpwm();

extern DigitalPin<BEAT_PIN_1> beat_pin;
extern DigitalPin<BEAT_PIN_2> tempo_pin;

static void inline __attribute__((always_inline)) set_status_leds_and_mask_within_interrupt(uint8_t new_portb_val, uint8_t new_portb_mask)
{
    portb_mask = new_portb_mask;
    portb_val = new_portb_val;
}

static void inline __attribute__((always_inline)) set_status_leds_within_interrupt(uint8_t new_portb_val)
{
    set_status_leds_and_mask_within_interrupt(new_portb_val, MASK_RESET_VAL);
}

static void inline __attribute__((always_inline)) clear_status_leds_within_interrupt()
{
    portb_mask = MASK_RESET_VAL;
    portb_val = 0;
}

static void inline __attribute__((always_inline)) set_status_leds_and_mask(uint8_t new_portb_val, uint8_t new_portb_mask)
{
    cli();
    set_status_leds_and_mask_within_interrupt(new_portb_val, new_portb_mask);
    sei();
}

static void inline __attribute__((always_inline)) set_status_leds(uint8_t new_portb_val)
{
    cli();
    set_status_leds_within_interrupt(new_portb_val);
    sei();
}

static void inline __attribute__((always_inline)) clear_status_leds()
{
    cli();
    clear_status_leds_within_interrupt();
    sei();
}

static void inline __attribute__((always_inline)) set_status_leds_and_mask_rotate(uint8_t new_portb_val, uint8_t new_portb_mask)
{
    cli();
    set_status_leds_and_mask_within_interrupt(new_portb_val, new_portb_mask);
    sei();
}

static void inline __attribute__((always_inline)) set_status_leds_front_buffer_only(uint8_t new_portb_val)
{
    cli();
    uint8_t temp = portb_val & 0xF0;
    portb_val = temp | (new_portb_val & 0x0F);
    sei();
}

static void inline __attribute__((always_inline)) set_status_leds_back_buffer_only(uint8_t new_portb_val)
{
    cli();
    uint8_t temp = portb_val & 0x0F;
    portb_val = temp | (new_portb_val & 0xF0);
    sei();
}


#endif /* _LEDPWM_H */
