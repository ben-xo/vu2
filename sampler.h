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
  byte samples[SAMP_BUFF_LEN];
};

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern Sampler sampler;
// extern byte samples[SAMP_BUFF_LEN];
extern byte beat_bitmap[SAMP_BUFF_LEN >> 3];
// extern volatile uint8_t current_sample;
extern uint8_t last_processed_sample_bd;
extern uint8_t last_processed_sample_vu;
extern volatile uint16_t sample_sum;

void setup_sampler(uint16_t timer_counter);
uint8_t calculate_vu(uint8_t sample_ptr, uint8_t *min_val_out, uint8_t *max_val_out, uint8_t vu_lookbehind);
uint8_t calculate_auto_gain_bonus(uint8_t vu_width);

void set_beat_at(uint8_t offset, bool is_beat);
bool get_beat_at(uint8_t offset);

__attribute__((always_inline)) uint8_t inline new_sample_count_since(uint8_t current_sample) {
  return (current_sample - last_processed_sample_vu) & ~SAMP_BUFF_LEN;
}

__attribute__((always_inline)) uint8_t inline new_sample_count() {
  return new_sample_count_since(sampler.current_sample);
}

__attribute__((always_inline)) uint8_t inline next_sample_index(uint8_t index) {
  return (index + 1) & ~SAMP_BUFF_LEN;
}

__attribute__((always_inline)) uint8_t inline consume_sample_index() {
  last_processed_sample_vu = next_sample_index(last_processed_sample_vu);
  return last_processed_sample_vu;
}

__attribute__((always_inline)) void inline sample()
{
//#ifdef DEBUG_SAMPLE_RATE
//  DEBUG_SAMPLE_RATE_PORT |= (1 << DEBUG_SAMPLE_RATE_PIN);
//#endif

 /** C version **/
  // ADCSRA = (1 << ADPS1) | (1 << ADEN) | (1 << ADSC); // trigger a sample. Spell out the settings to save clock cycles.
  // // we know what ADCSRA should be set to, so we can do this in 2 cycles instead of the 4 it would take with ADCSRA |= (1 << ADSC)
  
  // uint8_t sample_idx = (sampler.current_sample + 1) & ~SAMP_BUFF_LEN;
  // uint8_t *ss = sampler.samples;
  // sampler.current_sample = sample_idx;

  // byte sample = ADCH;
  // byte* the_sample = ss + sample_idx;
  // uint8_t old_sample_at_position = *the_sample;
  // sampler.sample_sum = sampler.sample_sum - old_sample_at_position + sample;
  // *the_sample = sample;

 /** assembler version of the above ^ **/

 asm volatile (

  // ADCSRA = (1 << ADPS1) | (1 << ADEN) | (1 << ADSC); // trigger a sample. Spell out the settings to save clock cycles.
  // // // we know what ADCSRA should be set to, so we can do this in 3 cycles instead of the 4 it would take with ADCSRA |= (1 << ADSC)
    "ldi r24, %[ADCSRA_val] ; 194 \t\n"
    "sts %[ADCSRA_addr], r24 \t\n"
  
    "ldi r30, lo8(%[ss]) \t\n"
    "ldi r31, hi8(%[ss]) \t\n"
    "ld r24, Z \t\n" // current_sample is offset 0
    "subi r24, 0xFF \t\n"
    "andi r24, 0x7F \t\n"
    "st Z, r24 \t\n"

    // instead of ldi / add / adc (which means we need a 0 reg), do add / brcc / subi (no reg needed)
    "add r30, r24 \t\n"
    // "ldi r25, 0x00 \t\n"
    // "adc r31, r25 \t\n"
    "brcc .+2 \t\n"
    "subi r31, 0xFF \t\n"

    "ldd r24, Z+1 \t\n"
    "out %[temp_reg], r24 \t\n"

    "lds r24, %[ADCH_addr] \t\n"
    "std Z+1, r24 \t\n"
    "lds r30, %[sample_sum]  \t\n"
    "lds r31, %[sample_sum]+0x1  \t\n"

    "add r30, r24 \t\n"
    // "ldi r24, 0x00 \t\n"
    // "adc r31, r24 \t\n"
    "brcc .+2 \t\n"
    "subi r31, 0xFF \t\n"

    "in r24, %[temp_reg] \t\n"
    "sub r30, r24 \t\n"
    "sbci r31, 0x00 \t\n"

    "sts %[sample_sum]+0x1, r31 \t\n"
    "sts %[sample_sum], r30 \t\n"
     :
     : 
     [ADCSRA_addr] "M" (_SFR_IO_ADDR(ADCSRA) + 0x20),
     [ADCSRA_val] "M" ((1 << ADPS1) | (1 << ADEN) | (1 << ADSC)),
     [ADCH_addr] "M" (_SFR_IO_ADDR(ADCH) + 0x20),
     [temp_reg] "I" (_SFR_IO_ADDR(EEARL)),  // abuse a register we're not using. Abusable reg's include: EEDR, EEARL
     [ss] "i" (&sampler.current_sample),
     [sample_sum] "i" (&sample_sum)
     :
     "r24", "r30", "r31"
   );

//#ifdef DEBUG_SAMPLE_RATE
//  DEBUG_SAMPLE_RATE_PORT &= ~(1 << DEBUG_SAMPLE_RATE_PIN);
//#endif
}

#endif /* _SAMPLER_H */
