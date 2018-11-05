#include "config.h"
#include "render.h"
#include "filter.h"
#include "sampler.h"
#include "ultrafastneopixel.h"

unsigned long time;

void setup() {
  // put your setup code here, to run once:

  // the pin with the push button
  pinMode(BUTTON_PIN, INPUT);
  pinMode(NEOPIXEL_PIN, OUTPUT);
  
  // the pin with the push-button LED
  pinMode(BUTTON_LED_PIN,OUTPUT);  

  // the pin with the mode display
  pinMode(MODE_LED_PIN_1,OUTPUT);
  pinMode(MODE_LED_PIN_2,OUTPUT);
  pinMode(MODE_LED_PIN_3,OUTPUT);
  pinMode(MODE_LED_PIN_4,OUTPUT);

  time = micros(); // Used to track rate

//  setup_filter();
//  setup_render();
//  setup_sampler();
}

void loop() {
  // put your main code here, to run repeatedly:
  UltraFastNeoPixel the_strip = UltraFastNeoPixel(STRIP_LENGTH);
  the_strip.begin();
  the_strip.clear();
  the_strip.show();
  while(true) {
//  the_strip.setPixelColor(0,255,0,0);
    the_strip.setPixelColor(3,0,0,255);
    the_strip.setPixelColor(4,255,0,0);
    the_strip.setPixelColor(5,0,255,0);
    the_strip.setPixelColor(6,0,0,0);
    the_strip.setPixelColor(7,0,0,0);
    the_strip.setPixelColor(8,0,0,0);
    the_strip.setPixelColor(50,128,64,0);
    the_strip.show();
    delay(5);
    the_strip.setPixelColor(3,0,0,0);
    the_strip.setPixelColor(4,0,0,0);
    the_strip.setPixelColor(5,0,0,0);
    the_strip.setPixelColor(6,255,0,255);
    the_strip.setPixelColor(7,255,255,0);
    the_strip.setPixelColor(8,0,255,255);
    the_strip.setPixelColor(50,0,64,128);
    the_strip.show();
    delay(5);
  }
}
