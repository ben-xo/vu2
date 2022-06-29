/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _GPIO0_H
#define _GPIO0_H

#define LEDPWM_BUFFER_SELECT_FLAG_BIT (2)
#define END_OF_FRAME_FLAG_BIT         (1)
#define EVERY_OTHER_FRAME_FLAG_BIT    (0)

#define LEDPWM_BUFFER_SELECT_FLAG (1<<LEDPWM_BUFFER_SELECT_FLAG_BIT)
#define END_OF_FRAME_FLAG         (1<<END_OF_FRAME_FLAG_BIT)
#define EVERY_OTHER_FRAME_FLAG    (1<<EVERY_OTHER_FRAME_FLAG_BIT)

#endif _GPIO0_H