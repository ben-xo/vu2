#include <Arduino.h>
#include "config.h"

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
unsigned char samples[32];
unsigned char current_sample = 0;

void setup_sampler() {

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
  
//  // Set ADC to 77khz, max for 10bit (this is for the beat detector)
//  bitSet(ADCSRA,ADPS2);
//  bitClear(ADCSRA,ADPS1);
//  bitClear(ADCSRA,ADPS0);

  // this sets up the timer1 interrupt for 5kHz.
  
//  cli();
//  //set timer1 interrupt at 5kHz
//  TCCR1A = 0; // set entire TCCR1A register to 0
//  TCCR1B = 0; // same for TCCR1B
//  TCNT1  = 0; //initialize counter value to 0
//  // set compare match register for 1hz increments
//  OCR1A = 3199; // = (16*10^6) / (1*5000) - 1 (must be <65536)
//  // turn on CTC mode
//  TCCR1B |= (1 << WGM12);
//  // enable timer compare interrupt
//  TIMSK1 |= (1 << OCIE1A);
//  sei();
}

//ISR(TIMER1_COMPA_vect){
//  samples[current_sample] = analogRead(AUDIO_INPUT);
//}


//ISR(ADC_vect)
//{
//  samples[current_sample] = ADCH;
//  current_sample++;
//  current_sample &= 31; // counter loops back to 0
//}

