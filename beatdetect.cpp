#include <Arduino.h>
#include "config.h"
#include "beatdetect.h"

void setup_beatdetect() {
    DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
    // PD2 (PCINT0 pin) is now an input

    PORTD |= (1 << PORTD2);    // turn On the Pull-up
    // PD2 is now an input with pull-up enabled

    EICRA |= (1 << ISC10);    // set INT0 to trigger on ANY logic change
    EIMSK |= (1 << INT1);     // Turns on INT0  
}

ISR (INT1_vect)
{
    /* interrupt code here */
    beats_from_interrupt = PIND & ((1 << BEAT_PIN_1) | (1 << BEAT_PIN_2)); // read once - port is volatile
}
