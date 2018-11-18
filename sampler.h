// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern volatile char samples[64];
extern volatile unsigned char current_sample;
void setup_sampler();
