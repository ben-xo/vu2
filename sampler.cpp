#include <Arduino.h>
#include "config.h"

unsigned int samples[16];
unsigned int current_sample = 0;

void setup_sampler() {

  // Set ADC to 77khz, max for 10bit (this is for the beat detector)
  bitSet(ADCSRA,ADPS2);
  bitClear(ADCSRA,ADPS1);
  bitClear(ADCSRA,ADPS0);

  // this sets up the timer1 interrupt for 5kHz.
  
  cli();
  //set timer1 interrupt at 5kHz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; //initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 3199; // = (16*10^6) / (1*5000) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

ISR(TIMER1_COMPA_vect){
  samples[current_sample] = analogRead(AUDIO_INPUT);
  current_sample++;
  current_sample &= 15; // counter loops back to 0
}
