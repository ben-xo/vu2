/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "sampler.h"

#include <FastLED.h> // for qsub8

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.

Sampler sampler = { 0 };
byte beat_bitmap[SAMP_BUFF_LEN >> 3] = { 0 };
uint8_t last_processed_sample_bd = 0;
uint8_t last_processed_sample_vu = 0;
volatile uint16_t sample_sum = 0; // (DC offset approximated by sample_sum / SAMP_BUFF_LEN)

/**
 * timer_counter = (F_CPU / (1 * desired_sample_frequency) - 1)
 * 
 * that "1" is the prescaler
 */
void setup_sampler(uint16_t timer_counter) {

  cli();
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  DIDR0 = (1<<AUDIO_INPUT_PIN);
  ADMUX |= (AUDIO_INPUT_PIN & 0x07)     // set A0 analog input pin
        |  (1 << REFS0)   // set reference voltage to AVCC
        |  (0 << REFS1)   // set reference voltage to AVCC s
        |  (1 << ADLAR)   // left align ADC value to 8 bits from ADCH register
  ;

  // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
  // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles

  ADCSRA  = 0
         | (1 << ADPS2) 
         | (1 << ADPS1) 
         | (0 << ADPS0)
  ; // fastest prescaler it can do. Most precise timing for ADC.

//ADCSRA |= (1 << ADATE) // enable auto trigger
//ADCSRA |= (1 << ADIE)  // enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN)  // enable ADC
//       |  (1 << ADSC)  // start ADC measurements
  ;


  // XXX previous versions of this code used TIMER1 for the sampler, but now we hook onto LEDPWM timer2.

  
  sei();
}

/**
 * N.B the actual sample() method is defined in the header file so that it can be hooked onto the ledpwm interrupt to share interrupts.
 * LEDPWM goes at 10khz and our usual sample frequency is exactly half that. That frees TIMER1 for other uses.
 */

uint8_t calculate_vu(uint8_t sample_ptr, uint8_t *min_val_out, uint8_t *max_val_out, uint8_t vu_lookbehind) {
  uint8_t max_val=0, min_val=255, i=0;
  do {
    uint8_t int_sample = sampler.samples[(uint8_t)(sample_ptr - i) & ~SAMP_BUFF_LEN];
    if(int_sample > max_val) max_val = int_sample;
    if(int_sample < min_val) min_val = int_sample;
    i++;
  } while(i < vu_lookbehind);

  *min_val_out = min_val;
  *max_val_out = max_val;
  return max_val - min_val;
}

uint8_t calculate_auto_gain_bonus(uint8_t vu_width) {
  // return a multiplier that will scale vu_width so that a "recently large" vu_width would be 255 (adjust to taste).
  // "recently large" means we track the largest seen VU width, but scale it down on every frame. "New loudness" will therefore increase this, but quiet patches will decrease it.
  static uint8_t weighted_max_vu = 0;
  const uint8_t scale = AUTOGAIN;
  static uint8_t scale_count = scale;
  if(scale_count == 0) {
    // decrease on each Nth call.
    // this represents the "headroom" between the widest recent vu_width and saturation.
    // we gradually tighten the headroom in order to bring out quiet sounds, but if the input
    // is larger than the max then this pushes it back up.
    weighted_max_vu = qsub8(weighted_max_vu, 1);
    scale_count = scale;
  } else {
    scale_count--;
  }
  if(vu_width > weighted_max_vu) {
    weighted_max_vu = vu_width;
  }

  return weighted_max_vu;
}

void set_beat_at(uint8_t offset, bool is_beat) {
  uint8_t beat_bitmap_index = offset / 8;
  uint8_t beat_bitmap_shift = offset & 7;
  if(is_beat) {
    beat_bitmap[beat_bitmap_index] |= (1 << beat_bitmap_shift);
  } else {
    beat_bitmap[beat_bitmap_index] &= ~(1 << beat_bitmap_shift);
  }
}

bool get_beat_at(uint8_t offset) {
  uint8_t beat_bitmap_index = offset / 8;
  uint8_t beat_bitmap_shift = offset & 7;
  return beat_bitmap[beat_bitmap_index] & (1 << beat_bitmap_shift);
}
