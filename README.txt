Sound Reactive Lights (visualiser!)

This is a self indulgent project by Ben XO https://github.com/ben-xo

The idea is that you feed line level audio into a pin on an arduino, and you get pretty lights out on a strip of Neopixels (WS2812 or compatible).

The basic hardware is simple:

* Arduino
* Strip of 60 Neopixels (but can be configured up to 100 before it gets too slow / runs out of RAM)
* 330ohm resister for Neopixels
* 10k resistor and a push button (for changing modes)
* another 10k resistor pull-down for the audio (or it's noisy)
* anything else is optional, but you might want a buffer cap for the line-in otherwise it can be quite noisy. There are more complicated configurations possible

How to use

Circuit

* very simmilar to https://elmwoodelectronics.ca/blogs/news/maker-festival-projects-giant-neopixel-vu-meter but I'll explain anyway (that diagram uses A1 instead of A0 for sampling)
* load up config.h as that's where the pins are assigned
* attach 5v power and 1uF decoupling cap to your strip
* attach the same 5v power to 5v and GND of arduino (or ignore this instruction but whatever you do, attach the GND of the strip to the Arduino GND!)
* attach the 330 resistor to digital output 6 (as in config.h) and the other end to the pixel strip signal in.
* attach a 10k resistor from A0 analog input to GND
* attach your line in GND to Arduino Analog GND
* attach your line in signal to A0
* connect BEAT_PIN_1 and BEAT_PIN_2 (pins 2 and 3) directly to GND (otherwise they'll float, and do weird shit)
* connect BUTTON_PIN (4) to 10k resistor then that to GND
* wire your push button to BUTTON_PIN (so it's pull-up when pushed, but GND via 10k when not)

the beat pins are designed to spice up the visualisers when low pass analog beats are sent to them. experiment...

Software

1) Set up Arduino Studio
2) Install FastLED lib
3) REPLACE FastLED lib with the version from https://github.com/ben-xo/FastLED . This version has modifications to enable interrupts in certain places in order to not drop samples.
4) Load vu3.ino into Arduino Studio
5) edit config.h . set your strip length. maybe comment out the DEBUG defines if i left them uncommented
6) program arduino

There is a screensaver which kicks in after 15 seconds silence, but when there's a signal it is back to VU. You might need to adjust the levels low or high! Sensitivity CAN be adjusted with care in sampler.cpp (or with resistors)

Push button to change modes.


Enhancements

* You can attach LEDs (output, LED to GND) to the LED pins mentioned in config.h. They will show a binary of which mode is currently in effect.
* if you hold down the button at start up it will enter test mode and the LEDs will be a simple level meter so you can test it before attaching. the Neopixel strip
* The output from the lights pin can drive two strips with the same data so you could put them back to back to make a VU twice as long reflected in the middle!
