#include <Arduino.h>
#include "config.h"
#include "sampler.h"

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
volatile char samples[64] __attribute__((__aligned__(256)));
volatile unsigned char current_sample = 0;

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
  samples[sample_idx/4] = sample;
  ++sample_idx;
  sample_idx &= (SAMP_BUFF_LEN * 4) - 1; // clamp to 255 (which is 4 * 64)
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


