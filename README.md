Sound Reactive Lights (visualiser!)
===================================

This is a self indulgent project by Ben XO https://github.com/ben-xo

The idea is that you feed line level audio into a pin on an arduino, and you get pretty lights out on a strip of Neopixels (WS2812 or compatible).

License
-------

Copyright Ben XO https://github.com/ben-xo All rights reserved.

THIS PROJECT IS NOT OPEN SOURCE. Whilst I have put the source code on GitHub, no part of this project is currently released under any open source license.

I reserve all rights and the right to change this license in future.

How to use
==========

The basic hardware is simple:

* Arduino
* Strip of 60 Neopixels (but can be configured up to 100 before it gets too slow / runs out of RAM)
* 330ohm resistor for Neopixels
* 10k resistor and a push button (for changing modes)
* another 10k resistor pull-down for the audio (or it's noisy)
* anything else is optional, but you might want a buffer cap for the line-in otherwise it can be quite noisy. There are more complicated configurations possible
* 5v power supply of your choice.

Notes on Power
--------------

Powering an Arduino and lights safely / effectively is a dark art.

Choose a PSU you trust. For 1 Arduino and 2x strips of 60 NeoPixels you probably don't need more than a 2A supply because we're never going to be turning all the lights on full white and leaving them on full white. See https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels for TMI about how to calculate power usage.

You can wire a 5v supply directly to the 5v pins on the Arduino, but if you do that, you bypass the fuses on the Arduino and if your PSU turns out to be untrustworthy you may end up needing to buy a new Arduino.

You can power the Arduino and lights separately, but if you do, you must conenct the GND pins together.

You'll know if you don't have enough power because the thing will keep rebooting (you get a nice rainbow every time it starts up)

Circuit
-------

First of all this should go without saying but LEAVE THE POWER DISCONNECTED until you're ready

Very simmilar (BUT NOT IDENTICAL) to https://elmwoodelectronics.ca/blogs/news/maker-festival-projects-giant-neopixel-vu-meter 

You should load up config.h as that's where the pins are assigned - and double check these instructions against it.

* attach 1uF decoupling cap across to your strip. (This stops the lights browning out the Arduino when they flash brightly.) This is a standard NeoPixel wiring.
* attach PSU GND to BOTH your NeoPixels AND the GND of arduino
* attach 5v of your PSU to BOTH your NeoPixels AND the GND of Arduino (OR ignore this and see "Notes On Power" above)
* attach the 330 resistor to digital output 6 (as in config.h) and the other end to the pixel strip "signal in".
* attach a 10k resistor from A0 analog input to GND (it's to prevent a noisy input when the audio is disconencted)
* attach your line-in GND to Arduino GND
* attach your line-in signal to A0
* connect BEAT_PIN_1 and BEAT_PIN_2 (pins 2 and 3) directly to GND (otherwise they'll float, and do weird shit). See "Enhancements" for using these for more fun purposes.
* connect BUTTON_PIN (4) to 10k resistor then that to GND (pull it down, otherwise it will float and press the button randomly.)
* wire your push button from 5v to the button, and then other site of the button to BUTTON_PIN (so it's pull-up when pushed).

The BEAT_PIN_* pins are designed to spice up the visualisers when low pass analog beats are sent to them. experiment...

Software
--------

1) Set up Arduino Studio
2) Install FastLED lib
3) REPLACE FastLED lib with the version from https://github.com/ben-xo/FastLED . My version has modifications to enable interrupts in certain places in order to not drop samples.
4) Load vu3.ino into Arduino Studio
5) edit config.h . set your strip length. maybe comment out the DEBUG defines if i left them uncommented
6) program the arduino over USB.

Finally
-------

Plug the audio in. Plug the power in. Adjust the volume...

Push button to change modes.

There is a screensaver which kicks in after 15 seconds silence, but when there's a signal it is back to VU. You might need to adjust the levels low or high! Sensitivity CAN be adjusted with care in sampler.cpp (or with resistors)


Tips
====

Mac owners can make open Audio MIDI Setup to create a virtual soundcard that combines both the speakers AND the line-out, so that you can play audio out loud AND into the lights simultaneously. Neat trick


Enhancements
============

* You can attach LEDs (output, LED to GND) to the LED pins mentioned in config.h. They will show a binary of which mode is currently in effect. (They're even properly PWM'd so you don't need to attach resistors, like the lazy builder you always wanted to be!)
* if you hold down the button at start up it will enter test mode and the LEDs will be a simple level meter so you can test it before attaching. the Neopixel strip
* The output from the lights pin can drive two strips with the same data so you could put them back to back to make a VU twice as long reflected in the middle!

About The Software
==================

Whilst this project has evolved (slowly) from some sketches I found online, the current code is entirely my own work.

How It Works
------------

The main loop of the code has three phases:
1) Processing samples (to calculate the VU), beat detection based on current "beat" input status, and see if the button was pushed to change modes.
2) Rendering patterns into the pattern buffer to be shown on the lights, based on those samples, and then displaying them
3) wasting whatever time remains to stay at 125FPS.

There are also a series of interrupts used to process inputs.
* We use TIMER1 set at 5kHz to trigger the onboard ADC to sample from line-in, and store it in a sample buffer. (This ensures we can catch up on missed samples, even though rendering the lights takes longer than 1 sample).
* We use TIMER2 and some very tight ASM code to PWM-flash the LEDs, so that they don't get hot and explode. (Miraculously, even at 20,000 interrupts a second, this takes <2% CPU time)
* We use INT0 and INT1 to record if we're currently in a "beat" or not. This is used for tempo detection or just to augment the visualisers. (To be honest this part isn't as good as i'd like).

Want to work on it?
-------------------

Yeah! You do! I have no idea what to tell you, ask me questions on Twitter @benxo I have loads of ideas and none of them are important.

Thanks for coming to my TED talk







