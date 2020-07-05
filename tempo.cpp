#include <Arduino.h>
#include "tempo.h"

#define BEAT_FLASH_COUNT (BEAT_FLASH_LENGTH * (SAMP_FREQ / 1000))

unsigned int rising_edge_times[16] = {0}; // millis
unsigned int rising_edge_gap[16] = {0};   // millis
unsigned int beat_gap_sum = 0; 
unsigned int beat_gap_avg = 0;            // millis
uint8_t edge_index = 0;

// at 125fps each frame is 8ms
uint16_t frames_to_beat_on      = 0;
uint16_t frames_to_beat_off     = 0;
uint16_t frames_to_cancel_tempo = 0;

// whether tempo calculation is ready yet
static bool cleared = true;

bool tempo_beat = false;

uint8_t on_for = 0;
 

unsigned int inline short_millis() {
  return (unsigned int)millis();
}

void setup_tempo() {

}

void clear_tempo() {
# ifdef SERIAL_DEBUG_TEMPO
   Serial.println("C");
# endif
  beat_gap_sum = 0;
  beat_gap_avg = 0;
  cleared = true;
  edge_index = 0;
  for(uint8_t i = 0; i < 16; i++) {
    rising_edge_gap[i] = 0;
  }
}

void record_rising_edge() {
  unsigned int now = short_millis();
  unsigned int gap;
  gap = now - rising_edge_times[edge_index];

  // 150ms is slightly less than a half beat at 180bpm ((60/180/2)*1000 == ~167ms)
  if(gap > 150) { // don't record beats which are too close together
    
    edge_index = (edge_index + 1) & 15; // range 0 to 15
    rising_edge_times[edge_index] = now;
    
    beat_gap_sum -= rising_edge_gap[edge_index];
    rising_edge_gap[edge_index] = gap;
    beat_gap_sum += gap;
    beat_gap_avg = beat_gap_sum/16;

    // only show tempo estimate after we have accrued enough beats to make it possibly accurate.
    if(cleared && edge_index == 15) {
      cleared = false;
      frames_to_beat_on = 0;
      frames_to_beat_off = (beat_gap_avg >> 1) >> FRAME_DIVISOR; // half the period, in frames.
#     ifdef SERIAL_DEBUG_TEMPO
        Serial.println("UC");
#     endif
    }
  }
}

uint8_t recalc_tempo() {
  // this method recalculates if there should be a change in the tempo signal status.
  // this method is called once every FRAME so (default) 125 times a second.

  // ** next clear ** 
  uint16_t clear_on = frames_to_cancel_tempo - 1;
  if(clear_on > frames_to_cancel_tempo) {
    // underflow: time has elapsed: too long without a beat
    clear_tempo();
  }
  frames_to_cancel_tempo = clear_on;

  if(cleared) {
    // silent for too long: lights off
    return TEMPO_FALL;
  }

  // decrement the next-flash and next off counters, flashing on or off if they underflow.
  // There's approx. 13s of headroom here at 5kHz (65536 * 200 microseconds = 13.1s) which means
  // that the slowest we can flash would be about 6.5 BPM without errors. 
  // However, we don't go that slow - a timeout should put us into "cleared" mode

  // ** next on ** 
  uint16_t beat_on = frames_to_beat_on - 1;
  if(beat_on > frames_to_beat_on) {
    // underflow: time has elapsed: rising edge

    on_for = 0;

    // estimate next flash times from tempo
    uint16_t last_edge = rising_edge_times[edge_index];
    uint16_t now = short_millis();
    uint16_t last_beat = 0; // now

    // if we're rising >1/8 beat from the last rising edge, assume we've drifted and adjust
    // TODO: verify this
    if(now - last_edge > (beat_gap_avg >> 3)) {
      last_beat = (last_edge - now); // this will underflow, but that's on purpose
    } else {
      frames_to_beat_on = 0;
    }
    
    frames_to_beat_on  = (last_beat + beat_gap_avg) >> FRAME_DIVISOR;
    frames_to_beat_off = ((beat_gap_avg >> 1) >> FRAME_DIVISOR);

#   ifdef SERIAL_DEBUG_TEMPO
      Serial.println(frames_to_beat_on);
#   endif

    return TEMPO_RISE;
  } else {
    frames_to_beat_on = beat_on;
  }

  // ** next off ** 
  uint16_t beat_off = frames_to_beat_off - 1;
  if(on_for < BEAT_FLASH_COUNT) {
      on_for++;
  }
  if(beat_off > frames_to_beat_off) {
    // underflow: time has elapsed: falling edge
#   ifdef SERIAL_DEBUG_TEMPO
      Serial.println("v");
#   endif

    frames_to_beat_on  = ((beat_gap_avg >> 1) >> FRAME_DIVISOR);
    frames_to_beat_off = beat_gap_avg >> FRAME_DIVISOR;
    return TEMPO_FALL;
  } else {
    frames_to_beat_off = beat_off;
  }

  if(beat_gap_avg >= MIN_BPM_MILLIS) {
    // turn off all the lights if BPM is too low
    return TEMPO_FALL;
  }

  if(on_for >= BEAT_FLASH_COUNT) {
    // turn off all the lights after 30ms (or whatever) so they pulse
    return TEMPO_FALL;
  }

  return TEMPO_NO_CHANGE;
}
