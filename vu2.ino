#include "config.h"
#include "render.h"
#include "filter.h"
#include "sampler.h"

unsigned long time;

void setup() {
  // put your setup code here, to run once:

  // the pin with the push button
  pinMode(BUTTON_PIN, INPUT);
  
  // the pin with the push-button LED
  pinMode(BUTTON_LED_PIN,OUTPUT);  

  // the pin with the mode display
  pinMode(MODE_LED_PIN_1,OUTPUT);
  pinMode(MODE_LED_PIN_2,OUTPUT);
  pinMode(MODE_LED_PIN_3,OUTPUT);
  pinMode(MODE_LED_PIN_4,OUTPUT);

  time = micros(); // Used to track rate

  setup_filter();
  setup_render();
  setup_sampler();
}

void loop() {
  // put your main code here, to run repeatedly:

}
