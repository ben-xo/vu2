// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern uint8_t samples[SAMP_BUFF_LEN];
extern volatile uint8_t current_sample;
extern volatile uint8_t min_seen_sample;
extern volatile uint8_t max_seen_sample;
extern volatile uint8_t new_sample_count;
void setup_sampler();
