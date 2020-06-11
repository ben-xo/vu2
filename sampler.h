/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

// sample buffer. this is written into by an interrupt handler serviced by the ADC interrupt.
extern byte samples[SAMP_BUFF_LEN];
extern volatile uint8_t current_sample;
extern volatile uint8_t new_sample_count;
void setup_sampler();
uint8_t calculate_vu(uint8_t sample_ptr, uint8_t *min_val_out, uint8_t *max_val_out);
