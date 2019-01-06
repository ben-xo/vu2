#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "sampler.h"

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
byte samples[SAMP_BUFF_LEN] __attribute__((__aligned__(256)));
volatile uint16_t current_sample = 0;
volatile uint8_t new_sample_count = 0;

void disable_timer0_interrupt() {
//  TIMSK0 &= ~_BV(TOIE0); // disable timer0 overflow interrupt. Breaks serial.
}

void setup_sampler() {

  // we don't want to trigger the default timer interrupt routine.
  // it's surprisingly heavy, and it would introduce jitter to the sampler.
  disable_timer0_interrupt();

  cli();
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  ADMUX |= (0 & 0x07)     // set A0 analog input pin
        |  (1 << REFS0)   // set reference voltage to internal 1.1v (gives a signal boost for audio).
        |  (1 << REFS1)   // set reference voltage to internal 1.1v (gives a signal boost for audio).
        |  (1 << ADLAR)   // left align ADC value to 8 bits from ADCH register
  ;

  // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
  // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles

  ADCSRA  = 0
//         | (1 << ADPS2) 
         | (1 << ADPS1) 
         | (1 << ADPS0)
  ; // 128 prescaler for 9600 Hz, which is the slowest it will do.

//ADCSRA |= (1 << ADATE) // enable auto trigger
//ADCSRA |= (1 << ADIE)  // enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN)  // enable ADC
//       |  (1 << ADSC)  // start ADC measurements
  ;

  TCCR1B = 0 | (1 << CS11) 
             | (1 << WGM12)
  ; // set up TIMER1 with no prescaler, and interrupt on overflow
  
  OCR1A = 3199; // overflow value for 5kHz
  TCNT1 = 0;
  TIMSK1 |= (1 << OCIE1A); // enable timer1
  
  sei();
}

ISR(TIMER1_COMPA_vect)
{
  ADCSRA |= (1 << ADSC); // trigger next analog sample.
  
  uint8_t sample_idx = (current_sample + 1) & ((SAMP_BUFF_LEN * 8) - 1); // clamp to the buffer size.
  current_sample = sample_idx;
  
  byte sample = ADCH;
  volatile byte* the_sample = samples + current_sample;
  *the_sample = sample;
  new_sample_count++;
}

//// This optimised version saves 12 cycles from the C code above. 
//// Not sure if it was worth it to save 12 cycles per sample, but it was fun…
//ISR(ADC_vect) __attribute__((naked));
//ISR(ADC_vect)
//{
//  volatile uint8_t* cs = &current_sample;
//  uint8_t* ss = samples;
//  
//  asm volatile (
//    "push r24 \n\t"
//    "in r24, 0x3f \n\t"
//    "push r24 \n\t"
//    "push  r30 \n\t"
//    "push  r31 \n\t"
//    "lds r30, %[cs] \n\t" // 0x800101 <current_sample>
//    "subi  r30, 0xFF \n\t" 
//    "sts current_sample, r30 \n\t" // 0x800101 <current_sample>
//    "lds r24, 0x0079 \n\t" // 0x800079 <__TEXT_REGION_LENGTH__+0x7e0079>
//    "ldi r31, 0x00 \n\t"
//    "asr r30 \n\t"
//    "subi  r31, 0xFD \n\t" // <samples> TOOD: i think this is an address?? careful!
//    "st  Z, r24 \n\t"
//    "pop r31 \n\t"
//    "pop r30 \n\t"
//    "pop r24 \n\t"
//    "out  0x3f, r24 \n\t" // restore status register
//    "pop r24 \n\t"
//    "reti \n\t"
//    :: 
//    [cs] "i" (cs),
//    [ss] "i" (ss)
//  );
//}


