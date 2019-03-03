/*-------------------------------------------------------------------------
  This file is a cross between the AdaFruit NeoPixel library, and the code
  found at https://github.com/bigjosh/SimpleNeoPixelDemo and (Originally
  https://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/ )

  I took it from there and modified it for my neopixel strips, which are really
  very relaxed with timing indeed.
  -------------------------------------------------------------------------*/

#include "ultrafastneopixel.h"

// Constructor when length, pin and type are known at compile-time:
UltraFastNeoPixel::UltraFastNeoPixel(uint16_t n) :
  pixels(NULL) 
{
  updateLength(n);
}

// via Michael Vogt/neophob: empty constructor is used when strand length
// isn't known at compile-time; situations where program config might be
// read from internal flash memory or an SD card, or arrive via serial
// command.  If using this constructor, MUST follow up with updateType(),
// updateLength(), etc. to establish the strand type, length and pin number!
UltraFastNeoPixel::UltraFastNeoPixel() :
  numLEDs(0), numBytes(0), pixels(NULL)
{
}

UltraFastNeoPixel::~UltraFastNeoPixel() {
  if(pixels)   free(pixels);
}

void UltraFastNeoPixel::begin(void) {
  bitSet( PIXEL_DDR , PIXEL_BIT );
}

void UltraFastNeoPixel::updateLength(uint16_t n) {
  if(pixels) free(pixels); // Free existing data (if any)

  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numBytes = sizeof(RGB8[n]);
  if((pixels = (Pixel *)malloc(numBytes))) {
    memset(pixels, 0, numBytes);
    numLEDs = n;
  } else {
    numLEDs = numBytes = 0;
  }
}

// Set pixel color from separate R,G,B components:
void UltraFastNeoPixel::setPixelColor(
 uint16_t n, uint8_t r, uint8_t g, uint8_t b) {

  if(n < numLEDs) {
    Pixel *p;
    p = &pixels[n];    // 3 bytes per pixel
    p->color.r = r;         
    p->color.g = g;
    p->color.b = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void UltraFastNeoPixel::setPixelColor(uint16_t n, uint32_t c) {
  if(n < numLEDs) {
    Pixel *p;
    p = &pixels[n];
    p->color.r = (uint8_t)(c >> 16),
    p->color.g = (uint8_t)(c >>  8),
    p->color.b = (uint8_t)c;
  }
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t UltraFastNeoPixel::Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t UltraFastNeoPixel::getPixelColor(uint16_t n) const {
  if(n >= numLEDs) return 0; // Out of bounds, return no color.

  Pixel *p;

  p = &pixels[n];
  // No brightness adjustment has been made -- return 'raw' color
  return ((uint32_t)p->color.r << 16) |
         ((uint32_t)p->color.g <<  8) |
          (uint32_t)p->color.b;
}

uint8_t* UltraFastNeoPixel::getPixelColorRGB(uint16_t n) const {
  Pixel* p;
  p = &pixels[n];
  return p->rgb;
}

// Returns pointer to pixels[] array.  Pixel data is stored in device-
// native format and is not translated here.  Application will need to be
// aware of specific pixel data format and handle colors appropriately.
Pixel *UltraFastNeoPixel::getPixels(void) const {
  return pixels;
}

uint16_t UltraFastNeoPixel::numPixels(void) const {
  return numLEDs;
}


void UltraFastNeoPixel::clear() {
  memset(pixels, 0, numBytes);
}

// NOTES: I have found that almost all of the delays are entirely unnecessary at 16MHz.
// We DO need T1H delay, and we DO need cli()/sei() around 0 bits
void sendBit( bool bitVal ) {

    if ( bitVal ) {      // 1-bit

      shortcli();
      asm volatile (
        "sbi %[port], %[bit] \n\t"        // Set the output bit
        ::
        [port]  "I" (_SFR_IO_ADDR(PIXEL_PORT)),
        [bit]   "I" (PIXEL_BIT)
      );

      // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      // No need to disable interrupts here - if anything, interrupts just make the gap longer.
      __builtin_avr_delay_cycles( NS_TO_CYCLES(T1H) - 2 ); 

      asm volatile (
        "cbi %[port], %[bit] \n\t"        // Clear the output bit
        ::
        [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
        [bit]   "I" (PIXEL_BIT)
      );
      // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      // No need to disable interrupts here - if anything, interrupts just make the gap longer.
      shortsei();

    } else {             // 0-bit

      // We must disable interrupts here. Otherwise, an interrupt might happen mid-bit
      // And flip the pixel from a 0 to a 1.

      shortcli();
      asm volatile (
        "sbi %[port], %[bit] \n\t"        // Set the output bit
        "nop \n\t"
        "nop \n\t"
        "cbi %[port], %[bit] \n\t"        // Clear the output bit
        ::
        [port]  "I" (_SFR_IO_ADDR(PIXEL_PORT)),
        [bit]   "I" (PIXEL_BIT)
      );
      shortsei();

    }
 
    // Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time)
    // Here I have been generous and not tried to squeeze the gap tight but instead erred on the side of lots of extra time.
    // This has the nice side effect of avoid glitches on very long strings
}
 
void UltraFastNeoPixel::sendByte( uint8_t the_byte ) {
    for( uint8_t the_bit = 0 ; the_bit < 8 ; the_bit++ ) { 
      sendBit( bitRead( the_byte , 7 ) ); // Neopixel wants bit in highest-to-lowest order
                                      // so send highest bit (bit #7 in an 8-bit byte since they start at 0)
      the_byte <<= 1; // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
    }
}

void UltraFastNeoPixel::sendPixel( Pixel p ) {
  sendByte(p.color.g); // Neopixel wants colors in green-then-red-then-blue order
  sendByte(p.color.r);
  sendByte(p.color.b);
}
 
// Just wait long enough without sending any bots to cause the pixels to latch and display the last sent frame
 
void UltraFastNeoPixel::show() {
  longcli();
  for(unsigned int i = 0; i < numBytes; i += 3) {
    sendPixel(pixels[i]);
  }
  longsei();
  _delay_us( (RES / 1000UL) + 1);        // Round up since the delay must be _at_least_ this long (too short might not work, too long not a problem)
}

/* A PROGMEM (flash mem) table containing 8-bit unsigned sine wave (0-255).
   Copy & paste this snippet into a Python REPL to regenerate:
import math
for x in range(256):
    print("{:3},".format(int((math.sin(x/128.0*math.pi)+1.0)*127.5+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM _sineTable[256] = {
  128,131,134,137,140,143,146,149,152,155,158,162,165,167,170,173,
  176,179,182,185,188,190,193,196,198,201,203,206,208,211,213,215,
  218,220,222,224,226,228,230,232,234,235,237,238,240,241,243,244,
  245,246,248,249,250,250,251,252,253,253,254,254,254,255,255,255,
  255,255,255,255,254,254,254,253,253,252,251,250,250,249,248,246,
  245,244,243,241,240,238,237,235,234,232,230,228,226,224,222,220,
  218,215,213,211,208,206,203,201,198,196,193,190,188,185,182,179,
  176,173,170,167,165,162,158,155,152,149,146,143,140,137,134,131,
  128,124,121,118,115,112,109,106,103,100, 97, 93, 90, 88, 85, 82,
   79, 76, 73, 70, 67, 65, 62, 59, 57, 54, 52, 49, 47, 44, 42, 40,
   37, 35, 33, 31, 29, 27, 25, 23, 21, 20, 18, 17, 15, 14, 12, 11,
   10,  9,  7,  6,  5,  5,  4,  3,  2,  2,  1,  1,  1,  0,  0,  0,
    0,  0,  0,  0,  1,  1,  1,  2,  2,  3,  4,  5,  5,  6,  7,  9,
   10, 11, 12, 14, 15, 17, 18, 20, 21, 23, 25, 27, 29, 31, 33, 35,
   37, 40, 42, 44, 47, 49, 52, 54, 57, 59, 62, 65, 67, 70, 73, 76,
   79, 82, 85, 88, 90, 93, 97,100,103,106,109,112,115,118,121,124};

/* Similar to above, but for an 8-bit gamma-correction table.
   Copy & paste this snippet into a Python REPL to regenerate:
import math
gamma=2.6
for x in range(256):
    print("{:3},".format(int(math.pow((x)/255.0,gamma)*255.0+0.5))),
    if x&15 == 15: print
*/
static const uint8_t PROGMEM _gammaTable[256] = {
  0,   3,   5,   7,   9,  11,  13,  14,  16,  18,  19,  21,  22,  24,  25,  26,
 28,  29,  31,  32,  33,  35,  36,  37,  39,  40,  41,  42,  44,  45,  46,  47,
 48,  50,  51,  52,  53,  54,  56,  57,  58,  59,  60,  61,  63,  64,  65,  66,
 67,  68,  69,  70,  71,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
 84,  85,  86,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100,
101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
117, 118, 119, 120, 121, 122, 123, 123, 124, 125, 126, 127, 128, 129, 130, 131,
132, 133, 134, 135, 136, 137, 138, 139, 140, 140, 141, 142, 143, 144, 145, 146,
147, 148, 149, 150, 151, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161,
161, 162, 163, 164, 165, 166, 167, 168, 169, 169, 170, 171, 172, 173, 174, 175,
176, 177, 177, 178, 179, 180, 181, 182, 183, 183, 184, 185, 186, 187, 188, 189,
190, 190, 191, 192, 193, 194, 195, 196, 196, 197, 198, 199, 200, 201, 202, 202,
203, 204, 205, 206, 207, 207, 208, 209, 210, 211, 212, 212, 213, 214, 215, 216,
217, 217, 218, 219, 220, 221, 222, 222, 223, 224, 225, 226, 227, 227, 228, 229,
230, 231, 232, 232, 233, 234, 235, 236, 236, 237, 238, 239, 240, 240, 241, 242,
243, 244, 245, 245, 246, 247, 248, 249, 249, 250, 251, 252, 253, 253, 254, 255};

uint8_t UltraFastNeoPixel::sine8(uint8_t x) const {
  return pgm_read_byte(&_sineTable[x]); // 0-255 in, 0-255 out
}

uint8_t UltraFastNeoPixel::gamma8(uint8_t x) const {
  return pgm_read_byte(&_gammaTable[x]); // 0-255 in, 0-255 out
}


