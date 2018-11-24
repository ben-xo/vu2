#include <Arduino.h>
#include "config.h"
#include "sampler.h"

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
uint8_t samples[SAMP_BUFF_LEN] __attribute__((__aligned__(256)));
volatile uint8_t current_sample = 255; // start at -1, because of the algorithm in ADC_vect
volatile uint8_t max_seen_sample = 0;
volatile uint8_t min_seen_sample = 255;

void disable_timer0_interrupt() {
  TIMSK0 &= ~_BV(TOIE0); // disable timer0 overflow interrupt
}

void setup_sampler() {

  // we don't want to trigger the default timer interrupt routine.
  // it's surprisingly heavy, and it would introduce jitter to the sampler.
  disable_timer0_interrupt();

  cli();
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  ADMUX |= (0 & 0x07)     // set A0 analog input pin
        |  (1 << REFS0)   // set reference voltage
        |  (1 << ADLAR)   // left align ADC value to 8 bits from ADCH register
  ;

  // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
  // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescaler for 9600 Hz, which is the slowest it will do.

  ADCSRA |= (1 << ADATE) // enable auto trigger
         |  (1 << ADIE)  // enable interrupts when measurement complete
         |  (1 << ADEN)  // enable ADC
         |  (1 << ADSC)  // start ADC measurements
  ;
  sei();
}

ISR(ADC_vect)
{
  // If the sample index is not a multiple of 2, we overwrite the same sample.
  // Why? Because the slowest we can sample with the builtin ADC interrupt is 9600Hz, but 
  // We don't really need to sample that fast so instead we throw away 1/2 samples.
  // I chose to do this rather than setting a separate timer interrupt, because there's no jitter in the ADC this way
  // Effective sample rate is 4800Hz
  
  uint8_t sample_idx = (current_sample + 1) & ((SAMP_BUFF_LEN * 8) - 1); // clamp to the buffer size.
  current_sample = sample_idx;
  
  uint8_t sample = ADCH;
  volatile uint8_t* the_sample = samples + (current_sample >> 1);
  if(sample < 2) sample = 0; // filter DC when there's no sound.
  *the_sample = sample;
}

//ISR(ADC_vect)
//{
//  volatile uint8_t* cs = &current_sample;
//  uint8_t* ss = samples;
//  
//  asm volatile (
//    "push  r24 \n\t"
//    "push  r30 \n\t"
//    "push  r31 \n\t"
//    "lds r30, %[cs] \n\t" // 0x800101 <current_sample>
//    "subi  r30, 0xFF \n\t" 
//    "sts current_sample, r30 \n\t" // 0x800101 <current_sample>
//    "lds r24, 0x0079 \n\t" // 0x800079 <__TEXT_REGION_LENGTH__+0x7e0079>
//    "ldi r31, 0x00 \n\t"
//    "asr r31 \n\t"
//    "ror r30 \n\t"
//    "sbci  r31, hi8(%[ss]) \n\t" // <samples>
//    "cpi r24, 0x02 \n\t"
//    "brcc  .+2 \n\t"
//    "ldi r24, 0x00 \n\t"
//    "st  Z, r24 \n\t"
//    "pop r31 \n\t"
//    "pop r30 \n\t"
//    "pop r24 \n\t"
//    :: 
//    [cs] "i" (cs),
//    [ss] "i" (ss)
//  );
//}


