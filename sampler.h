// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern volatile char samples[SAMP_BUFF_LEN];
extern volatile unsigned char current_sample;
void setup_sampler();
