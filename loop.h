/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _LOOP_H
#define _LOOP_H

#include <Arduino.h>
#include "config.h"
#include "sampler.h"

#include "tempo.h"

#include "framestate.h"

#include "buttons.h"
#include "hardreset.h"
#include "sober.h"

#include "fps.h"
#include "debug.h"

// the loop is inlined, but filter_beat needs continuity.
extern bool filter_beat;
extern uint8_t my_current_sample;
extern uint16_t my_sample_sum;

__attribute__((always_inline)) static void one_frame_sample_handler() {

    DEBUG_FRAME_RATE_HIGH();
    DEBUG_SAMPLE_RATE_LOW();
        
    // read these as they're volatile
    my_current_sample = sampler.current_sample;
    uint8_t my_new_sample_count = (my_current_sample - last_processed_sample_bd) & ~SAMP_BUFF_LEN;

    DEBUG_SAMPLE_RATE_HIGH();

    // now let's do some beat calculations


    bool was_beat = filter_beat;

    bool is_beat_1 = false; // start calculation assuming no beat in this frame

    uint8_t my_sample_base = my_current_sample - my_new_sample_count;
    uint8_t offset = 0;
    uint8_t sample_idx;
    do {
      sample_idx = (my_sample_base + offset) & ~SAMP_BUFF_LEN;
      uint8_t val = sampler.samples[sample_idx];
      // Serial.println(val);
      filter_beat = PeckettIIRFixedPoint(val, filter_beat);
      set_beat_at(sample_idx, filter_beat);
      offset++;

      // If there was a beat edge detected at any point, set is_beat_1.
      // This gives a 1 frame resolution on beats, which is 8ms resolution at 125fps - good enough for us.
      // If we only checked the end of the frame, we might miss a beat that was very short.
      is_beat_1 |= filter_beat;
    } while(offset < my_new_sample_count);
    last_processed_sample_bd = sample_idx;


    F.is_beat_1 = is_beat_1;
    if(!was_beat && F.is_beat_1) {
        record_rising_edge();
    }

    F.is_beat_2 = recalc_tempo(F.is_beat_2);

#ifdef BEAT_WITH_INTERRUPTS
    // this won't be much use unless you also rip out the IIR code below…
    byte is_beats = beats_from_interrupt;
    F.is_beat_1 = is_beats & (1 << BEAT_PIN_1);
    F.is_beat_2 = is_beats & (1 << BEAT_PIN_2);
#endif

    F.min_vu = 0;
    F.max_vu = 255;

    // read these again, as the sampler probably sampled some stuff during the beat processing
    cli();
    my_current_sample = sampler.current_sample;
    my_sample_sum = sample_sum;
    sei();
    my_new_sample_count = (my_current_sample - last_processed_sample_vu) & ~SAMP_BUFF_LEN;
    last_processed_sample_vu = my_current_sample;


    // Currently, the lookbehind for the VU is always the number of samples queued up since the last VU (i.e. a whole Frame's worth)
    // With 5kHz sample rate and 125fps, this is usually 40 samples. But because the interrupts are staggered so they don't all fire at once,
    // occassionally it's 39 or 41.
#ifndef VU_LOOKBEHIND
    F.vu_width = calculate_vu(my_current_sample, &F.min_vu, &F.max_vu, my_new_sample_count);
#else
    F.vu_width = calculate_vu(my_current_sample, &F.min_vu, &F.max_vu, VU_LOOKBEHIND);
#endif

    uint8_t recent_max_vu = calculate_auto_gain_bonus(F.vu_width);
    F.vu_width = F.vu_width + scale8(F.vu_width, 255 - recent_max_vu);

    if (F.pushed || F.vu_width > ATTRACT_MODE_THRESHOLD) {
      // loudness: cancel attract mode, and so does a button press.
      F.is_silent = false;
      F.is_attract_mode = false;
    } else {
      // quiet: short or long?
      if(!F.is_silent) {
        // first loop of silence. Record time.
        silent_since = start_time; // note start time of silence
        F.is_silent = true;
      } else {
        // 2nd+ loop of silence. Long enough for attract mode?
        if (!F.is_attract_mode && ((start_time - silent_since)/1024 > ATTRACT_MODE_TIMEOUT)) {
          F.is_attract_mode = true;
        }
      }
    }

    DEBUG_SAMPLE_RATE_LOW();
}

static void frame_epilogue() {
    DEBUG_SAMPLE_RATE_LOW();
    DEBUG_FRAME_RATE_LOW();
    reach_target_fps();
    F.frame_counter++;
}

#endif /* _LOOP_H */
