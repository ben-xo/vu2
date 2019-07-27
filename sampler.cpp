#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "sampler.h"

#define TIMER_COUNTER (F_CPU / (1 * SAMP_FREQ) - 1)

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
byte samples[SAMP_BUFF_LEN] __attribute__((__aligned__(256)));
volatile uint8_t current_sample = 0;
volatile uint8_t new_sample_count = 0;

void setup_sampler() {

  cli();
  ADCSRA = 0;             // clear ADCSRA register
  ADCSRB = 0;             // clear ADCSRB register
  ADMUX |= (AUDIO_INPUT_PIN & 0x07)     // set A0 analog input pin
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

  // TIMER 1 for interrupt frequency 5000 Hz:  
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 109; // initialize counter value to 0
  // set compare match register for 5000 Hz increments (configured in config.h)
  OCR1A = TIMER_COUNTER; // = 16000000 / (1 * 5000) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

#ifdef DEBUG_SAMPLE_RATE
  // debugging pin for checking sample rate
  pinMode (DEBUG_SAMPLE_RATE_PIN, OUTPUT);
#endif
  
  sei();
}

ISR(TIMER1_COMPA_vect)
{
#ifdef DEBUG_SAMPLE_RATE
  DEBUG_SAMPLE_RATE_PORT |= (1 << DEBUG_SAMPLE_RATE_PIN);
#endif

  ADCSRA |= (1 << ADSC); // trigger next analog sample.
  
  uint8_t sample_idx = (current_sample + 1);
  current_sample = sample_idx;
  
  byte sample = ADCH;
  volatile byte* the_sample = samples + current_sample;
  *the_sample = sample;
  new_sample_count++;
  
#ifdef DEBUG_SAMPLE_RATE
  DEBUG_SAMPLE_RATE_PORT &= ~(1 << DEBUG_SAMPLE_RATE_PIN);
#endif
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


uint8_t calculate_vu(uint8_t sample_ptr, uint8_t *min_val_out, uint8_t *max_val_out) {
  // VU is always width of last 20 samples, wherever we happen to be right now.
  uint8_t max_val=0, min_val=255;
  for (uint8_t i = 0; i < VU_LOOKBEHIND; i++) {
    uint8_t int_sample = samples[(sample_ptr-i)%SAMP_BUFF_LEN];
    if(int_sample > max_val) max_val = int_sample;
    if(int_sample < min_val) min_val = int_sample;
  }
  *min_val_out = min_val;
  *max_val_out = max_val;
  return max_val - min_val;
}
