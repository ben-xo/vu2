#ifndef DEBUG_ONLY

#include "effects.h"

void setup_render();
void rainbowCycle(uint8_t);
void render(unsigned int peakToPeak, bool is_beat, bool do_fade, byte mode, bool is_beat_2, uint8_t sample_ptr, uint8_t min_vu, uint8_t max_vu);
void do_banner();
void render_attract();

// and now: the actual effects

#include "fire.h"
#include "plume.h"

#endif

// modes 0 to MAX_MODE are effects
#define MAX_MODE 9
