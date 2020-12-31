#include <Arduino.h>
#include "tempo.h"

uint16_t rising_edge_times[16] = {0}; // frames (worth 8ms each at 125fps)
uint16_t rising_edge_gap[16] = {0};   // frames. TOOD: work out if this worth 32 bytes of RAM, or if it would be better to calculate these on demand (used only in record_rising_edge!)

uint8_t edge_index = 0;
uint16_t beat_gap_sum = 0; // frames
uint16_t beat_gap_avg = 0; // frames

uint16_t next_on_frame = 0;
uint16_t next_off_frame = 0;

static bool cleared = true;

void setup_tempo() {

}

void clear_tempo() {
  cleared = true;
  edge_index = 0;
}

// records the rising edge of a beat, filtering edges that are too close together
void record_rising_edge() {

  uint16_t gap = frame_counter - rising_edge_times[edge_index];

  // 160ms is slightly less than a half beat at 180bpm ((60/180/2)*1000 == ~167ms)
  if(gap > (160 / FRAME_LENGTH_MILLIS)) { // don't record beats which are too close together

    edge_index = (edge_index + 1) & 15; // range 0 to 15

    rising_edge_times[edge_index] = frame_counter;
    
    beat_gap_sum -= rising_edge_gap[edge_index];
    rising_edge_gap[edge_index] = gap;
    beat_gap_sum += gap;
    beat_gap_avg = beat_gap_sum/16;

    // Only show tempo estimate after we have accrued enough beats to make it possibly accurate.
    // Bear in mind that some of the times might be from the last "run" of beats before we cleared.
    // If we used them in the average, then the average would start off far too slow!
    // (We still do the average calculation, above, because it keeps the code simple)
    if(cleared && edge_index == 0) {
      cleared = false;
      next_on_frame = frame_counter;
      next_off_frame = frame_counter + BEAT_FLASH_LENGTH;
    }
  }
}


// This method is called once every FRAME
// The return value is 1 if frame has a tempo high, and 0 if the frame has a tempo low.
// Heuristic:
// If we're NOT cleared, but we haven't seen a real beat in a while, we assume the calculation is probably invalid and clear the tempo
// Then we see if this frame is a transition to ON or OFF. If so, we update the next transition times.
// Then, if we're currently "cleared", we return OFF (assuming that not enough beats have been recorded to predict the tempo)
// However, if we're NOT cleared, we return our calculated ON or OFF.
//
// bool is_tempo_output_high: if we were flashing an LED, was it on or off? (we only change this value at the on-off boundaries, otherwise leaving it the same)
bool recalc_tempo(bool is_tempo_output_high) {
    if (!cleared && (frame_counter - rising_edge_times[edge_index]) > (beat_gap_avg * 4) ) {
        // we will miss four beats before we get scared and stop the tempo
        clear_tempo();
        return false;
    }

    // basic on/off flash to the tempo
    if(frame_counter == next_on_frame) {
        // TODO: drift adjustment?
        next_on_frame = next_on_frame + beat_gap_avg;
        is_tempo_output_high = true;
    } else if(frame_counter == next_off_frame) {
        next_off_frame = next_off_frame + beat_gap_avg;
        is_tempo_output_high = false;
    }

    // don't flash tempos that are too slow.
    if(beat_gap_avg >= MIN_BPM_FRAMES) {
        // turn off all the lights if BPM is too low
        is_tempo_output_high = false;
    }

    return is_tempo_output_high;
}
