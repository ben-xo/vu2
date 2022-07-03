/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _GPIO0_H
#define _GPIO0_H

// - bit 1 is used as an overflow flag for the fps_count() to tick the FPS (see fps.h, fps_count.h)
// - bit 0 is used to trigger the sampler at half the PWM rate (e.g. 5kHz sampler for 10kHz PWM) (see ledpwm.h, sampler.h)

#define LEDPWM_ROTATE_BACK_BUFFER_FLAG_BIT (2)
#define END_OF_FRAME_FLAG_BIT              (1)
#define EVERY_OTHER_FRAME_FLAG_BIT         (0)

#define LEDPWM_ROTATE_BACK_BUFFER_FLAG (1<<LEDPWM_ROTATE_BACK_BUFFER_FLAG_BIT)
#define END_OF_FRAME_FLAG              (1<<END_OF_FRAME_FLAG_BIT)
#define EVERY_OTHER_FRAME_FLAG         (1<<EVERY_OTHER_FRAME_FLAG_BIT)

#endif _GPIO0_H
