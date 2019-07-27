#include <Arduino.h>
#include "fps.h"

uint32_t start_time = 0; // time each loop started.
uint32_t silent_since = 0; // time we've been silent since.
bool slow = false; // track render time

uint32_t last_delay=0;

void reach_target_fps() {
  uint32_t end_time = micros();
#ifdef FRAME_RATE_LIMIT
  uint32_t total_time = end_time - start_time;
  // XXX: for some reason the frame rate isn't stable. This stabilises it, even though it's the wrong FPS
  last_delay = (FRAME_LENGTH_MICROS - total_time) + (last_delay/2);
  delayMicroseconds(last_delay);
//  Serial.println(total_time);
#endif
  start_time = end_time;
}
