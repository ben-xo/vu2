#include <Arduino.h>
#include "config.h"

#include "ledpwm.h"
#include "sampler.h"
#include "beatdetect.h"
#include "buttons.h"
#include "fps.h"

#include "debugrender.h"

extern uint32_t start_time;
extern uint32_t silent_since;

void setup_debug();
void debug_loop();
