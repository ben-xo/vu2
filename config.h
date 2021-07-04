/*
 * Copyright Ben XO https://github.com/ben-xo All rights reserved.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/*** Pin configuration ***/
#define AUDIO_INPUT_PIN 0
#define BUTTON_PIN 4
#define NEOPIXEL_PIN 6
#define DUTY_CYCLE_LED 7
#define MODE_LED_PIN_1 8
#define MODE_LED_PIN_2 9
#define MODE_LED_PIN_3 10 
#define MODE_LED_PIN_4 11
#define MODE_LED_PIN_5 12
#define BUTTON_LED_PIN 13

/*** Optional beat detection using interrupts (would require a bit of a rework of the main ino) ***/
//#define BEAT_WITH_INTERRUPTS 1
#define BEAT_PIN_1 A1 // must be an interrupt pin e.g. 2 if BEAT_WITH_INTERRUPTS is 1
#define BEAT_PIN_2 A2 // must be an interrupt pin e.g. 3 if BEAT_WITH_INTERRUPTS is 1

/*** LED PWM configuration ***/
#define PWM_LED_FRQ      10000 // 10kHz
#define PWM_DUTY_PERCENT 10

/*** If you connect A, B, G and F of a 7 segment display to PORTB (that is, D8-D11) then you can have a cute shape instead of a binary counter ***/
#define SEVEN_SEG_MODE_DISPLAY 1


/*** Neopixel configuration ***/
#define STRIP_LENGTH 60 // don't recommend >100
#define FPS 150 // you can get up to about 180fps with a 60 strip, 125 with a 100-strip
#define FRAME_LENGTH_MICROS (1000000 / FPS) // 8000us
#define FRAME_LENGTH_MILLIS (1000 / FPS) // 8000us
#define FRAME_LENGTH_CYCLES (F_CPU / FPS)   // 128000 @ 16MHz
#define FRAME_RATE_LIMIT 1 // limitless is fun!

/*** Audio sampling config ***/
#define SAMP_BUFF_LEN 128 // this needs to be a power of 2. Also, if it's not 256, we get glitches!
#define SAMP_FREQ 5000 // Hz

/*** Beat Detect config ***/
#define AUTO_BEATS 128 // beats before change
#define AUTO_BEATS_MIN_THRESH 300 // ms
#define AUTO_BEATS_SILENCE_THRESH 5000 // ms
//#define BEAT_SUSTAIN 40 // minimum length of a beat detection

// This is the beat detect threshold.
// If you build a box without the pot, you can read the threshold out
// from one which has the pot using one of the test modes...
#define THRESHOLD_INPUT 1
#define DEFAULT_THRESHOLD 48.0
#define USE_POT_FOR_THRESHOLD 0


// This awkward calculation is to get it to round to nearest number of frames, which is roughly 4 frames at 125fps
// See https://stackoverflow.com/a/2422722. 
#define BEAT_FLASH_LENGTH ((30 + (1000/FPS) - 1) / (1000/FPS)) // frames
#define MIN_BPM_FRAMES ((FPS*60)/85) // frames per beat @ ~85bpm


/*** Attract mode config ***/
#define ATTRACT_MODE_THRESHOLD 8 // vu value
#define ATTRACT_MODE_TIMEOUT 15000 // ms (although this is compared 1024)
#define ATTRACT_MODE_DOTS 5

// By default it uses an entire frame's worth of samples, i.e everything not yet included in a VU.
// But you can override that to suit your taste, especially as upping the FPS would reduce the number of samples in the calculation.
//#define VU_LOOKBEHIND 20


/*** random debug stuff ***/
//#define DEBUG_ONLY 1
#define DEBUG_SAMPLE_RATE 1
#define DEBUG_SAMPLE_RATE_PORT PORTD
#define DEBUG_SAMPLE_RATE_PIN PD2
#define DEBUG_FRAME_RATE 1
#define DEBUG_FRAME_RATE_PORT PORTD
#define DEBUG_FRAME_RATE_PIN PD3

//#define LONGCLI 1

#endif /* _CONFIG_H */
