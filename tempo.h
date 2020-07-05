#ifndef _TEMPO_H
#define _TEMPO_H

#include <DigitalIO.h>
#include "config.h"

#define TEMPO_NO_CHANGE 0
#define TEMPO_RISE 1
#define TEMPO_FALL 2

void setup_tempo();
void clear_tempo();
void record_rising_edge();
uint8_t recalc_tempo();

extern bool tempo_beat;

#endif // _TEMPO_H
