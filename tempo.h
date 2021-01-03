#ifndef _TEMPO_H
#define _TEMPO_H

#include <DigitalIO.h>
#include "config.h"
#include "framestate.h"

void setup_tempo();
void clear_tempo();
void record_rising_edge();
bool recalc_tempo(bool is_tempo_output_high);

extern bool tempo_beat;
extern Framestate F;

#endif // _TEMPO_H
