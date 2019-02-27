#define AUDIO_INPUT 0
#define AUDIO_INPUT_LP 2
#define AUDIO_INPUT_HP 3
#define NEOPIXEL_PIN 6
#define DUTY_CYCLE_LED 7
#define BUTTON_PIN 4
#define BEAT_PIN_1 2
#define BEAT_PIN_2 3
#define BUTTON_LED_PIN 13
#define MODE_LED_PIN_1 9
#define MODE_LED_PIN_2 10
#define MODE_LED_PIN_3 11 
#define MODE_LED_PIN_4 12
#define STRIP_LENGTH 60
#define SAMP_BUFF_LEN 256 // this needs to be a power of 2.
#define SAMP_FREQ 5000 // Hz
#define AUTO_BEATS 64 // beats before change
#define AUTO_BEATS_MIN_THRESH 300 // ms
#define AUTO_BEATS_SILENCE_THRESH 5000 // ms
#define ATTRACT_MODE_THRESHOLD 8 // vu value
#define ATTRACT_MODE_TIMEOUT 15000 // ms (although this is compared 1024)
#define ATTRACT_MODE_DOTS 5
#define VU_LOOKBEHIND 10
//#define FRAME_RATE_LIMIT 1


// This is the beat detect threshold.
// If you build a box without the pot, you can read the threshold out
// from one which has the pot using one of the test modes...
#define THRESHOLD_INPUT 1
#define DEFAULT_THRESHOLD 48.0
#define USE_POT_FOR_THRESHOLD 0

//#define LONGCLI 1

