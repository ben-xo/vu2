#include <Arduino.h>
#include "config.h"
#include "sampler.h"

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
volatile unsigned char samples[SAMP_BUFF_LEN] __attribute__((__aligned__(256)));
volatile unsigned char current_sample = 0;
volatile unsigned char max_seen_sample = 0;
volatile unsigned char min_seen_sample = 255;

void setup_sampler() {

  cli();
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  ADMUX |= (0 & 0x07);    // set A0 analog input pin
  ADMUX |= (1 << REFS0);  // set reference voltage
  ADMUX |= (1 << ADLAR);  // left align ADC value to 8 bits from ADCH register

  // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
  // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles

  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescaler for 9600 Hz, which is the slowest it will do.

  ADCSRA |= (1 << ADATE); // enable auto trigger
  ADCSRA |= (1 << ADIE);  // enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN);  // enable ADC
  ADCSRA |= (1 << ADSC);  // start ADC measurements
  sei();
}

ISR(ADC_vect)
{
  unsigned char sample = ADCH;
  unsigned char sample_idx = current_sample;
  if(sample < 2) sample = 0; // filter DC when there's no sound.

  // If the sample index is not a multiple of 4, we overwrite the same sample.
  // Why? Because the slowest we can sample with the builtin ADC interrupt is 9600Hz, but 
  // We don't really need to sample that fast so instead we throw away 3/4 samples.
  // I chose to do this rather than setting a separate timer interrupt, because there's no jitter in the ADC this way
  // Effective sample rate is 2400Hz
  samples[sample_idx/8] = sample;
  ++sample_idx;
  sample_idx &= (SAMP_BUFF_LEN * 8) - 1; // clamp to the buffer size.
  current_sample = sample_idx;
}

//ISR(ADC_vect)
//{
//  volatile unsigned char* cs = &current_sample;
//  
//  asm volatile (
//    "push  r24 \n\t"
//    "push  r25 \n\t"
//    "push  r30 \n\t"
//    "push  r31 \n\t"
//    "lds r24, %[cs] \n\t"
//    "lds r25, 0x79 \n\t"
//    "mov r30, r24 \n\t"
//    "lsr r30 \n\t"
//    "ldi r31, 0x00 \n\t"
//    "subi r30, 0x00 \n\t"
//    "subi r31, 0xFE \n\t"
//    "st Z, r25 \n\t"
//    "subi r24, 0xFF \n\t"
//    "andi r24, 0x3F \n\t"
//    "sts %[cs], r24 \n\t"
//    "pop r31 \n\t"
//    "pop r30 \n\t"
//    "pop r25 \n\t"
//    "pop r24 \n\t"
//    :: [cs] "i" (cs)
//  );
//}


