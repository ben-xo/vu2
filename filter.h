#include <FIR.h>

void setup_filter();
float bassFilter(float sample);
float envelopeFilter(float sample);
float beatFilter(float sample);
extern FIR lowPass;
extern FIR highPass;
