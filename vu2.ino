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
#ifdef DEBUG
  Serial.begin(2000000);
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
  UltraFastNeoPixel the_strip = UltraFastNeoPixel(STRIP_LENGTH);
  the_strip.begin();
  the_strip.clear();
  the_strip.show();
#ifdef DEBUG
  uint8_t serial_i = 0;
#endif
  uint8_t sample_ptr = current_sample;

  while(true) {
    for (uint8_t j = 0; j < STRIP_LENGTH; j++) {
      // the +1 and +2 just make it a bit more colourful on the strip…
      the_strip.setPixelColor(j, 
        samples[(sample_ptr + j) % SAMP_BUFF_LEN], 
        samples[(sample_ptr + j*3) % SAMP_BUFF_LEN], 
        samples[(sample_ptr + j*5) % SAMP_BUFF_LEN]
       );
    }
#ifdef DEBUG
    i++;
    if(i % 10 == 0) {
      Serial.print("\n");
      Serial.print((uint8_t)samples[sample_ptr % SAMP_BUFF_LEN]);
      Serial.print(" ");
      Serial.print((uint8_t)samples[(sample_ptr-1) % SAMP_BUFF_LEN]);
      Serial.print(" ");
      Serial.print((uint8_t)samples[(sample_ptr-2) % SAMP_BUFF_LEN]);
      Serial.print(" ");
      Serial.print(millis());
      i = 0;
    }
 #endif
    the_strip.show();
  }
}
