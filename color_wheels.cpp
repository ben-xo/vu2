/*
 * I assert no copyright on this file because to be frank most of it came from the AdaFruit NeoPixel examples
 */

#include "color_wheels.h"

// originally taken from AdaFruit NeoPixel strandtest, then modified for FastLED

CRGB Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

CRGB Wheel2(byte WheelPos) {
  // 0 is blue (0,0,255)
  // 255 is yellow (255,127,0)
  CRGB col;
  col.r = WheelPos;
  col.g = WheelPos >> 1;
  col.b = 255-WheelPos;
  return col;
}

CRGB Wheel3(byte WheelPos) {
  CRGB col;
  col.r = 0;
  col.g = 128-WheelPos > 0 ? 128-WheelPos : 0;
  col.b = WheelPos > 128 ? 128 : WheelPos;
  return col;
}

CRGB Wheel_Purple_Yellow(byte WheelPos) {
  // 0 is purple (63,0,255)
  // 255 is yellow (255,127,0)
  CRGB col;
  col.r = map8(WheelPos,63,255);
  col.g = WheelPos >> 2;
  col.b = 255-WheelPos;
  return col;
}
