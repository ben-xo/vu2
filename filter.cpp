#include <Arduino.h>
#include "config.h"
#include "filter.h"
#include <FIR.h>

FIR lowPass = FIR();
FIR highPass = FIR();

static float lowPassCoeffs[] = {
0.058941394880239056,
0.067400917530924781,
0.074796392092324623,
0.080855873165259634,
0.085354127494019216,
0.088122662998354445,
0.089057263677756390,
0.088122662998354445,
0.085354127494019216,
0.080855873165259634,
0.074796392092324623,
0.067400917530924781,
0.058941394880239056
};

static float highPassCoeffs[] = {
0.014626551404255866,
0.067123004554128479,
0.067975933892614401,
-0.014742802807857579,
-0.159650634117853740,
-0.297951941959363475,
0.645239778068152026,
-0.297951941959363475,
-0.159650634117853740,
-0.014742802807857579,
0.067975933892614401,
0.067123004554128479,
0.014626551404255866
};

void setup_filter() {
  lowPass.setCoefficients(lowPassCoeffs);
  lowPass.setGain(1.0);
  highPass.setCoefficients(highPassCoeffs);
  highPass.setGain(1.0);
}

// stuff for the beat detector

// 20 - 200hz Single Pole Bandpass IIR Filter
float bassFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 9.1f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7960060012f * yv[0]) + (1.7903124146f * yv[1]);
    return yv[2];
}

// 10hz Single Pole Lowpass IIR Filter
float envelopeFilter(float sample) { //10hz low pass
    static float xv[2] = {0,0}, yv[2] = {0,0};
    xv[0] = xv[1]; 
    xv[1] = sample / 160.f;
    yv[0] = yv[1]; 
    yv[1] = (xv[0] + xv[1]) + (0.9875119299f * yv[0]);
    return yv[1];
}

// 1.7 - 3.0hz Single Pole Bandpass IIR Filter
float beatFilter(float sample) {
    static float xv[3] = {0,0,0}, yv[3] = {0,0,0};
    xv[0] = xv[1]; xv[1] = xv[2]; 
    xv[2] = sample / 7.015f;
    yv[0] = yv[1]; yv[1] = yv[2]; 
    yv[2] = (xv[2] - xv[0])
        + (-0.7169861741f * yv[0]) + (1.4453653501f * yv[1]);
    return yv[2];
}

bool beat_detect(float &envelope) {
    float beat, thresh;
    
    // Filter out repeating bass sounds 100 - 180bpm
    beat = beatFilter(envelope);

    // Threshold is based on potentiometer on AN1
#if USE_POT_FOR_THRESHOLD
    thresh = 0.02f * (float)analogRead(THRESHOLD_INPUT);
#else
    thresh = 0.02f * DEFAULT_THRESHOLD;
#endif

    // If we are above threshold, FOUND A BEAT
    if(beat > thresh) {
      return true;
    }
    return false;
}
