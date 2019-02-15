// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern byte samples[SAMP_BUFF_LEN];
extern volatile uint16_t current_sample;
extern volatile uint8_t new_sample_count;
void setup_sampler();

