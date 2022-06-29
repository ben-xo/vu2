/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "debug.h"

void setup_debug()
{
    cli();
#ifdef DEBUG_SAMPLE_RATE_PIN
    debug_sample_rate_pin.mode(INPUT_PULLUP);
#endif

#ifdef DEBUG_FRAME_RATE_PIN
    debug_frame_rate_pin.mode(INPUT_PULLUP);
#endif

#ifdef DEBUG_AUDIO_PROCESSING_RATE_PIN
    debug_audio_processing_rate_pin.mode(INPUT_PULLUP);
#endif

    sei();
}
