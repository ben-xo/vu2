extern class UltraFastNeoPixel strip;
void setup_render();
void rainbowCycle(uint8_t);
void render(unsigned int peakToPeak, bool is_beat, bool do_fade, char mode, unsigned int lpvu, unsigned int hpvu);

