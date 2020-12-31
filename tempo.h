#ifndef _TEMPO_H
#define _TEMPO_H

#include <DigitalIO.h>
#include "config.h"

void setup_tempo();
void clear_tempo();
void record_rising_edge();
bool recalc_tempo(bool is_tempo_output_high);

extern bool tempo_beat;
extern uint16_t frame_counter;

#endif // _TEMPO_H
