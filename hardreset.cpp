/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#include "hardreset.h"

void hard_reset()
{
    wdt_enable(WDTO_15MS);
    while(true) {
        asm("");
    }
}

