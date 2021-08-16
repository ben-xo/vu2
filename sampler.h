/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _SAMPLER_H
#define _SAMPLER_H

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"

#include "PeckettIIRFixedPoint.h"

#define SAMPLER_TIMER_COUNTER_FOR(desired_sample_frequency) ((F_CPU / (1 * desired_sample_frequency) - 1))

struct Sampler {
  volatile uint8_t current_sample;
  volatile uint8_t current_sample_val;
  uint8_t last_processed_sample;
  volatile uint16_t sample_sum; // (DC offset approximated by sample_sum / SAMP_BUFF_LEN)
  byte samples[SAMP_BUFF_LEN];
  byte beat_bitmap[SAMP_BUFF_LEN >> 3];
};

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern Sampler sampler;
// extern byte samples[SAMP_BUFF_LEN];
// extern byte beat_bitmap[SAMP_BUFF_LEN >> 3];
// extern volatile uint8_t current_sample;
// extern volatile uint8_t last_processed_sample;
// extern volatile uint16_t sample_sum;

void setup_sampler(uint16_t timer_counter);
uint8_t calculate_vu(uint8_t sample_ptr, uint8_t *min_val_out, uint8_t *max_val_out, uint8_t vu_lookbehind);
uint8_t calculate_auto_gain_bonus(uint8_t vu_width);

void set_beat_at(uint8_t offset, bool is_beat);
bool get_beat_at(uint8_t offset);

void __inline__ sample()
{
//#ifdef DEBUG_SAMPLE_RATE
//  DEBUG_SAMPLE_RATE_PORT |= (1 << DEBUG_SAMPLE_RATE_PIN);
//#endif

 /** C version **/
  // ADCSRA = (1 << ADPS1) | (1 << ADEN) | (1 << ADSC); // trigger a sample. Spell out the settings to save clock cycles.
  // // we know what ADCSRA should be set to, so we can do this in 2 cycles instead of the 4 it would take with ADCSRA |= (1 << ADSC)
  
  // uint8_t sample_idx = (current_sample + 1) & ~SAMP_BUFF_LEN;
  // current_sample = sample_idx;
  
  // byte sample = ADCH;
  // byte* the_sample = samples + sample_idx;
  // uint8_t old_sample_at_position = *the_sample;
  // sample_sum = sample_sum - old_sample_at_position + sample;
  // *the_sample = sample;

 /** assembler version of the above ^ **/
 uint8_t* ss = sampler.samples;

 asm volatile (

  // ADCSRA = (1 << ADPS1) | (1 << ADEN) | (1 << ADSC); // trigger a sample. Spell out the settings to save clock cycles.
    "ldi r24, %[ADCSRA_val] ; 194 \t\n"
    "sts %[ADCSRA_addr], r24 \t\n"
  // // // we know what ADCSRA should be set to, so we can do this in 2 cycles instead of the 4 it would take with ADCSRA |= (1 << ADSC)
  
  // // uint8_t sample_idx = (current_sample + 1) & ~SAMP_BUFF_LEN;
    "lds r30, current_sample \t\n"
    "subi  r30, 0xFF ; 255 \t\n"
    "andi  r30, 0x7F ; 127 \t\n"
  // // current_sample = sample_idx;
    "sts current_sample, r30 ; \t\n"
  
  // // byte sample = ADCH;
    "lds r18, %[ADCH_addr] \t\n"
  // // byte* the_sample = samples + sample_idx;
    "ldi r31, 0x00 ; 0 \t\n"
    "subi  r30, lo8(-(%[ss])) ; 128 \t\n"
    "sbci  r31, hi8(-(%[ss])) ; 252 \t\n"
  // // uint8_t old_sample_at_position = *the_sample;
  // // sample_sum = sample_sum - old_sample_at_position + sample;
    "lds r24, sample_sum  \t\n"
    "lds r25, sample_sum+0x1  \t\n"
    "add r24, r18 \t\n"
    "adc r25, r1 \t\n"
    "ld  r19, Z \t\n"
    "sub r24, r19 \t\n"
    "sbc r25, r1 \t\n"
    "sts sample_sum+0x1, r25 \t\n"
    "sts sample_sum, r24 \t\n"
  // // *the_sample = sample;
    "st  Z, r18 \t\n"
  // // new_sample_count++;
    "lds r24, new_sample_count \t\n"
    "subi  r24, 0xFF  \t\n"
    "sts new_sample_count, r24  \t\n"
     :
     : 
     [ADCSRA_addr] "M" (_SFR_IO_ADDR(ADCSRA) + 0x20),
     [ADCSRA_val] "M" ((1 << ADPS1) | (1 << ADEN) | (1 << ADSC)),
     [ADCH_addr] "M" (_SFR_IO_ADDR(ADCH) + 0x20),
     [ss] "i" (ss)
     :
     "r24", "r30", "r31", "r18", "r25", "r19"
   );

//#ifdef DEBUG_SAMPLE_RATE
//  DEBUG_SAMPLE_RATE_PORT &= ~(1 << DEBUG_SAMPLE_RATE_PIN);
//#endif
}

#endif /* _SAMPLER_H */
