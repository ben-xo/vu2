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
  setup_sampler();
  Serial.begin(2000000);
}

void loop() {
  // put your main code here, to run repeatedly:
  UltraFastNeoPixel the_strip = UltraFastNeoPixel(STRIP_LENGTH);
  the_strip.begin();
  the_strip.clear();
  the_strip.show();
  unsigned char i = 0;
  
  while(true) {
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
      // the +1 and +2 just make it a bit more colourful on the strip…
      the_strip.setPixelColor(j, samples[j % 32], samples[(j+1) % 32], samples[(j+2) % 32]);
    }
    if(i%2==0) {
      Serial.print("\n");
      Serial.print((uint8_t)samples[current_sample]);
    }
    i++;
    the_strip.show();
  }
}
