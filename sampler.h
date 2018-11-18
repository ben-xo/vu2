// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern volatile unsigned char samples[SAMP_BUFF_LEN];
extern volatile unsigned char current_sample;
extern volatile unsigned char min_seen_sample;
extern volatile unsigned char max_seen_sample;
void setup_sampler();
