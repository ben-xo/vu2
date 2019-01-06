#include "ultrafastneopixel.h"
#include "sampler.h"

void debug_render_combo(UltraFastNeoPixel *the_strip, bool is_beat, bool is_beat_2, uint8_t sample_ptr);
void debug_render_is_beat(UltraFastNeoPixel *the_strip, bool is_beat_1, bool is_beat_2);
void debug_render_samples(UltraFastNeoPixel *the_strip, uint8_t sample_ptr, bool colourful);
void debug_render_vu(UltraFastNeoPixel *the_strip, uint8_t vu_width);
