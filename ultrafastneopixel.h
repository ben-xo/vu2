/*--------------------------------------------------------------------
  Bastardized from Adafruit_Neopixel.h
  --------------------------------------------------------------------*/

#include "config.h"

#ifndef ULTRAFASTNEOPIXEL_H
#define ULTRAFASTNEOPIXEL_H

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#define PIXEL_PORT PORTB // Port of the pin the pixels are connected to
#define PIXEL_DDR DDRB // Port of the pin the pixels are connected to
#define PIXEL_BIT NEOPIXEL_PIN // Bit of the pin the pixels are connected to
 

// These are the timing constraints taken mostly from the WS2812 datasheets
// These are chosen to be conservative and avoid problems rather than for maximum throughput 
 
#define T1H  900    // Width of a 1 bit in ns
#define T1L  600    // Width of a 1 bit in ns
 
#define T0H  400    // Width of a 0 bit in ns
#define T0L  900    // Width of a 0 bit in ns
 
#define RES 7000    // Width of the low gap between bits to cause a frame to latch
 
// Here are some convenience defines for using nanoseconds specs to generate actual CPU delays
 
#define NS_PER_SEC (1000000000L) // Note that this has to be SIGNED since we want to be able to check for negative values of derivatives
 
#define CYCLES_PER_SEC (F_CPU)
 
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )
 
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )
 
#define DELAY_CYCLES(n) ( ((n)>0) ? __builtin_avr_delay_cycles( n ) : __builtin_avr_delay_cycles( 0 ) ) // Make sure we never have a delay less than zero

class UltraFastNeoPixel {

 public:

  // Constructor: number of LEDs
  UltraFastNeoPixel(uint16_t n);
  UltraFastNeoPixel(void);
  ~UltraFastNeoPixel();

  void
    begin(void),
    show(void),
    setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
    setPixelColor(uint16_t n, uint32_t c),
    clear(),
    updateLength(uint16_t n),
    sendBit(bool) __attribute__ ((optimize(0))),
    sendByte(unsigned char byte),
    sendPixel(unsigned char, unsigned char, unsigned char);
  uint8_t
   *getPixels(void) const,
    sine8(uint8_t) const,
    gamma8(uint8_t) const;
  uint16_t
    numPixels(void) const;
  static uint32_t
    Color(uint8_t r, uint8_t g, uint8_t b);
  uint32_t
    getPixelColor(uint16_t n) const;

 protected:

  uint16_t
    numLEDs,       // Number of RGB LEDs in strip
    numBytes;      // Size of 'pixels' buffer below (3 or 4 bytes/pixel)
  uint8_t
   *pixels;        // Holds LED color values (3 or 4 bytes each)    
};

#endif // ULTRAFASTNEOPIXEL_H
