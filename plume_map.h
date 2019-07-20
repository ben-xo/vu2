#include "config.h"

#define FADE_PLUME_FACTOR 51.0
#define FADE_PLUME_SCALE FADE_PLUME_FACTOR / ((float)STRIP_LENGTH/2)

// Macro expansion for building tables with sizes of powers of 2
#define S4(i)    S1((i)),   S1((i)+1),     S1((i)+2),     S1((i)+3)
#define S16(i)   S4((i)),   S4((i)+4),     S4((i)+8),     S4((i)+12)
#define S64(i)   S16((i)),  S16((i)+16),   S16((i)+32),   S16((i)+48)
#define S256(i)  S64((i)),  S64((i)+64),   S64((i)+128),  S64((i)+192)
#define S1024(i) S256((i)), S256((i)+256), S256((i)+512), S256((i)+768)

#define BS4(i)    BS1((i)),   BS1((i)+1),     BS1((i)+2),     BS1((i)+3)
#define BS16(i)   BS4((i)),   BS4((i)+4),     BS4((i)+8),     BS4((i)+12)
#define BS64(i)   BS16((i)),  BS16((i)+16),   BS16((i)+32),   BS16((i)+48)
#define BS256(i)  BS64((i)),  BS64((i)+64),   BS64((i)+128),  BS64((i)+192)
#define BS1024(i) BS256((i)), BS256((i)+256), BS256((i)+512), BS256((i)+768)

# define BS1(i) ((uint8_t) (-(i * FADE_PLUME_SCALE) + FADE_PLUME_FACTOR))
# define S1(i) ((uint8_t) ((i - (STRIP_LENGTH/2)) * FADE_PLUME_SCALE)) 


/** STATIC LOOKUP TABLES FOR PLUME SCALING **/

#if STRIP_LENGTH > 128
static const uint8_t PROGMEM fade_pixel_plume_map[128] = {
    BS64(0), BS64(64)
};
static const uint8_t PROGMEM fade_pixel_plume_map2[128] = {
    S64(STRIP_LENGTH/2), S64(STRIP_LENGTH/2 + 64)
};

#elif STRIP_LENGTH > 64
static const uint8_t PROGMEM fade_pixel_plume_map[64] = {
    BS64(0)
};
static const uint8_t PROGMEM fade_pixel_plume_map2[64] = {
    S64(STRIP_LENGTH/2)
};

#elif STRIP_LENGTH > 32
static const uint8_t PROGMEM fade_pixel_plume_map[32] = {
    BS16(0), BS16(16)
};
static const uint8_t PROGMEM fade_pixel_plume_map2[32] = {
    S16(STRIP_LENGTH/2), S16(STRIP_LENGTH/2 + 16)
};

#elif STRIP_LENGTH > 16
static const uint8_t PROGMEM fade_pixel_plume_map[16] = {
    BS16(0),
};
static const uint8_t PROGMEM fade_pixel_plume_map2[16] = {
    S16(STRIP_LENGTH/2)
};

#else 
static const uint8_t PROGMEM fade_pixel_plume_map[8] = {
    BS4(0), BS4(4)
};
static const uint8_t PROGMEM fade_pixel_plume_map2[8] = {
    S4(STRIP_LENGTH/2), S4(STRIP_LENGTH/2 + 4)
};

#endif
