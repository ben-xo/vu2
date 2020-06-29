/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */
 
#include <Arduino.h>
#include "config.h"
#include "sampler.h"

#include "PeckettIIRFixedPoint.h"

static int16_t thresh_f = 3000;

// Our Global Sample Rate, 5000hz
#define SAMPLEPERIODUS 200

// 20 - 200hz Single Pole Bandpass IIR Filter
// INPUT: 8-bit signed sample
// OUTPUT: 16 bit signed filtered sample
static int16_t bassFilter(int8_t sample) {
    const int32_t Qin = 16; // sample was only sampled with 8 bit precision, so we have 24 bits to play with after the point - but need some headroom too.
    const int32_t Qhalf = 8; // do some of the maths with headroom (thanks arithmetic) 
    const int32_t Qout = 4; // return higher precision, and hope the output hasn't clipped.
    const int32_t alpha1 = (-0.7960060012f * (1L << Qin));
    const int32_t alpha2 = (1.7903124146f * (1L << Qin));
  
    static int8_t xv[3] = {0L,0L,0L};
    static int32_t yv[3] = {0L,0L,0L};
    xv[0] = xv[1];
    xv[1] = xv[2]; 
    xv[2] = sample;
  
    yv[0] = yv[1]; 
    yv[1] = yv[2]; 

    int32_t x2_minus_x0 = ((int32_t)xv[2] - (int32_t)xv[0]) << (Qin - Qout);
    
    int32_t yv2;
    yv2 = x2_minus_x0;
    yv2 += ( alpha1 * (yv[0]) >> Qhalf );
    yv2 += ( alpha2 * (yv[1]) >> Qhalf );

    yv[2] = yv2 >> Qhalf;

    return (yv[2]); // this is being cast down to 16 bits, but there's enough headroom.
}

// 10hz Single Pole Lowpass IIR Filter
// INPUT: 16-bit signed sample
// OUTPUT: 16 bit signed filtered sample
static int16_t envelopeFilter(int16_t sample) { //10hz low pass
    const int32_t Qin = 16; // sample was only sampled with 8 bit precision, so we have 24 bits to play with after the point - but need some headroom too.
    const int32_t Qhalf = 8; // do some of the maths with headroom (thanks arithmetic) 
    const int32_t alpha3 = (0.9875119299f * (1L << Qin));

    static int16_t xv[2] = {0,0};
    static int32_t yv[2] = {0,0};
    xv[0] = xv[1];

    // XXX: PeckettIIR does some scaling here, I've just done no scaling and it SEEMS TO BE OKAY
    xv[1] = sample;
    
    yv[0] = yv[1];
    
    int32_t x0_plus_x1 =  ((int32_t)xv[0] + (int32_t)xv[1]);
    
    yv[1] = x0_plus_x1
           + (alpha3 * (yv[0]) >> Qhalf);

    yv[1] = yv[1] >> Qhalf;

    // manually convert from 32-bit fixed-point to 16-bit int
    // (this generates assembler about 2x as fast as the right-rotate alone)
    
    // XXX: this 11 was found by tuning. Need to derive it...
    // return yv[1] >> 11;
    
    return yv[1]; // once again output will be 8/8 fixed point compared to the original input
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
static int16_t beatFilter(int16_t sample) {
    const int32_t Qin = 8;
    const int32_t Qhalf = 4;
    const int32_t Qout = 2;
    const int32_t alpha4 = (int32_t)(-0.7169861741f * (1L << Qin));
    const int32_t alpha5 = (int32_t)(1.4453653501f  * (1L << Qin));
    
    static int16_t xv[3] = {0L,0L,0L};
    static int32_t yv[3] = {0L,0L,0L};
    
    xv[0] = xv[1]; 
    xv[1] = xv[2]; 
    xv[2] = sample;
    
    yv[0] = yv[1]; 
    yv[1] = yv[2];

    int32_t x2_minus_x0 =  (uint32_t)((int16_t)xv[2] - (int16_t)xv[0]) << (Qin - Qout);

    int32_t yv2;
    yv2 = x2_minus_x0;
    yv2 += ( alpha4 * (yv[0]) >> Qhalf );
    yv2 += ( alpha5 * (yv[1]) >> Qhalf );

    yv[2] = yv2 >> Qhalf;

    return yv[2];
}

void PeckettIIRFixedPointSetup() {
  setup_sampler(SAMPLER_TIMER_COUNTER_FOR(5000 /* Hz */));
}

void PeckettIIRFixedPoint(uint8_t val, bool *is_beat) {
    int8_t sample;
    int16_t value, envelope, beat;
    static uint8_t i = 200;
    
    // Read ADC and center so +-512
    sample = val-120;
    // Filter only bass component
    value = bassFilter(sample);

    // Take signal amplitude and filter
    if(value < 0)value=-value;
    envelope = envelopeFilter(value);

    // Every 200 samples (25hz) filter the envelope 
    if(i == 0) {
      // Filter out repeating bass sounds 100 - 180bpm
      beat = beatFilter(envelope);
//        Serial.println(beat);

      // If we are above threshold, light up LED
      if (beat > thresh_f) {
        *is_beat = true;
      } else {
        *is_beat = false;
      }
      
      //Reset sample counter
      i = 200;
    } else {
      i--;
    }
}
