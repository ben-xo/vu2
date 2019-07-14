#include <Arduino.h>
#include "config.h"
#include "beatdetect.h"

void setup_beatdetect() {
    DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
    DDRD &= ~(1 << DDD3);     // Clear the PD3 pin

//    PORTD |= (1 << PORTD2);    // turn On the Pull-up
//    PORTD |= (1 << PORTD3);    // turn On the Pull-up

    EICRA |= (1 << ISC10);    // set INT1 to trigger on ANY logic change
    EICRA |= (1 << ISC00);    // set INT1 to trigger on ANY logic change
    EIMSK |= (1 << INT0);     // Turns on INT0  
    EIMSK |= (1 << INT1);     // Turns on INT1 
}

ISR (INT1_vect, ISR_ALIASOF(INT0_vect));
ISR (INT0_vect)
{
    /* interrupt code here */
    beats_from_interrupt = PIND & ((1 << BEAT_PIN_1) | (1 << BEAT_PIN_2)); // read once - port is volatile
}
