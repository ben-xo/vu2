extern class UltraFastNeoPixel strip;
extern byte samples[SAMP_BUFF_LEN];
extern volatile uint16_t current_sample;
void setup_render();
void rainbowCycle(uint8_t);
void render(unsigned int peakToPeak, bool is_beat, bool do_fade, byte mode, unsigned int lpvu, unsigned int hpvu, bool is_beat_2, uint8_t sample_ptr);
void do_banner();
void render_attract();

