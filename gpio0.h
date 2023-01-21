/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _GPIO0_H
#define _GPIO0_H

// - bit 2 when set,   the mask is rotated on even frames, and only applied on odd frames. i.e front:back ratio 3:1
//         when clear, the mask is rotated on every frame, and applied on every frame      i.e front:back ratio 1:1
// - bit 1 is used as an overflow flag for the fps_count() to tick the FPS (see fps.h, fps_count.h)
// - bit 0 is used to trigger the sampler at half the PWM rate (e.g. 5kHz sampler for 10kHz PWM) (see ledpwm.h, sampler.h)

#define LEDPWM_MASK_DIV_2_FLAG_BIT         (2)
#define END_OF_FRAME_FLAG_BIT              (1)
#define EVERY_OTHER_FRAME_FLAG_BIT         (0)

#define LEDPWM_MASK_DIV_2_FLAG             (1<<LEDPWM_MASK_DIV_2_FLAG_BIT)
#define END_OF_FRAME_FLAG                  (1<<END_OF_FRAME_FLAG_BIT)
#define EVERY_OTHER_FRAME_FLAG             (1<<EVERY_OTHER_FRAME_FLAG_BIT)

#endif _GPIO0_H

/*

Mask=01010101

frame   LEDPWM_MASK_DIV_2_FLAG      EVERY_OTHER_FRAME_FLAG      mask LSB    BUFFER
0       0                           0                           1           back
1       0                           1                           0           front
2       0                           0                           1           back
3       0                           1                           0           front
4       0                           0                           1           back
5       0                           1                           0           front
6       0                           0                           1           back
7       0                           1                           0           front

frame   LEDPWM_MASK_DIV_2_FLAG      EVERY_OTHER_FRAME_FLAG      mask LSB    BUFFER
0       1                           0                           1           front // odd frame: mask ignored
1       1                           1                           1           back
2       1                           0                           0           front // odd frame: mask ignored
3       1                           1                           0           front
4       1                           0                           1           front // odd frame: mask ignored
5       1                           1                           1           back
6       1                           0                           0           front // odd frame: mask ignored
7       1                           1                           0           front 


rotate when:
!LEDPWM_MASK_DIV_2_FLAG OR  !EVERY_OTHER_FRAME_FLAG
swap when:
!LEDPWM_MASK_DIV_2_FLAG AND !EVERY_OTHER_FRAME_FLAG


Mask=00000001

frame   LEDPWM_MASK_DIV_2_FLAG      EVERY_OTHER_FRAME_FLAG      mask LSB    BUFFER
0       0                           0                           1           back
1       0                           1                           0           front
2       0                           0                           0           front
3       0                           1                           0           front
4       0                           0                           0           front
5       0                           1                           0           front
6       0                           0                           0           front
7       0                           1                           0           front

frame   LEDPWM_MASK_DIV_2_FLAG      EVERY_OTHER_FRAME_FLAG      mask LSB    BUFFER
0       1                           0                           1           front // odd frame: mask ignored
1       1                           1                           1           back
2       1                           0                           0           front // odd frame: mask ignored
3       1                           1                           0           front
4       1                           0                           0           front // odd frame: mask ignored
5       1                           1                           0           front
6       1                           0                           0           front // odd frame: mask ignored
7       1                           1                           0           front 


*/


