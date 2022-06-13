Sound Reactive Lights (visualiser!)
===================================

This is a self indulgent project by Ben XO https://github.com/ben-xo

The idea is that you feed line level audio into a pin on an arduino, and you get pretty lights out on a strip of Neopixels (WS2812 or compatible).


License
-------

Copyright Ben XO https://github.com/ben-xo All rights reserved.

THIS PROJECT IS NOT OPEN SOURCE. Whilst I have put the source code on GitHub, no part of this project is currently released under any open source license.

I reserve all rights and the right to change this license in future.

What does it look like?
=======================

Kinda like this https://www.instagram.com/p/Bt-8dVqlZSi/ or this https://www.instagram.com/p/Bt-8dVqlZSi/

Features
========

* Works from line-in
* Interrupt driven audio sampling - even with WS2812s (requires a slightly modified FastLED library at https://github.com/ben-xo/FastLED)
* A solid 180 frame per second with 60 WS2812s (or 125fps with 100 WS2812s) with no slow frames or dropped samples
* Beat detection and tempo estimation in realtime. Yep! All on a single Arduino
* 8 cool sound-reactive visualisers. These have been endlessly tinkered with so that they look okay at a range of input volumes and aren't TOO jarring when they change
* Automatic mode changing after a certain number of beats
* Effects work well for a range of music from house and techno to drum & bass
* Visualisers cope with things like DC Offset on the line-in
* push button to change modes
* Status LEDs driven using interrupt driven PWM (so no need for resistors on the status LEDs)
* Screensaver when the audio goes quiet. Because why not!
* Nice rainbow when you turn it on

How to use
==========


Circuit
-------

First of all this should go without saying but LEAVE THE POWER DISCONNECTED until you're ready

You should load up config.h as that's where the pins are assigned - and double check these instructions against it.

Also, I refuse to draw an ASCII diagram so you'll have to interpret my words.

Audio Input
-----------

The audio input stage expects "0" to be at around 2.5v (i.e the mid range of the analog input).

Minimum would be something like:
* ground of audio connected to ground of arduino
* signal (left or right audio channel) connected to A0 through a small capacitor e.g. 3.3uF
* A0 also connected to ground and +5v through 100k resistors (to create a potential divider to get the signal to sit at 2.5v)

That's it.

Output LEDs
-----------

* connect 5 LEDs from D8, D9, D10, D11 and D12 to ground. (no resistor needed). These show the current mode using either binary or seven seg "shapes"
* connect 2 LEDs from A1 and A2 to ground. These flash in time for the beats. I use the decimal point and base of the seven seg.

Mode Button
-----------

* Pull D4 to ground through a 10k+ resistor (if you don't and it floats the mode will go haywire)
* Connect a push button from D4 to pull up to 5v. That's it

LED light strip
---------------

Connect some WS2812s or whatever through a 220 or 330 resistor to D6, in "the usual way".


Notes on Power
--------------

Powering an Arduino and lights safely / effectively is a dark art.

Choose a PSU you trust. For 1 Arduino and 2x strips of 60 NeoPixels you probably don't need more than a 2A supply because we're never going to be turning all the lights on full white and leaving them on full white. See https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels for TMI about how to calculate power usage.

You can wire a 5v supply directly to the 5v pins on the Arduino, but if you do that, you bypass the fuses on the Arduino and if your PSU turns out to be untrustworthy you may end up needing to buy a new Arduino.

You can power the Arduino and lights separately, but if you do, you must conenct the GND pins together.

You'll know if you don't have enough power because the thing will keep rebooting (you get a nice rainbow every time it starts up)


Debug monitoring
----------------

If you wanna watch the frame rate (and time taken to do various stages in the main loop) on an oscilloscope, you should pull D2 and D3 to ground via 10k resistors and then hook your scope up to those pins.

Software
--------

1) Set up Arduino Studio (or arduino-cli)
2) Install FastLED lib
3) REPLACE FastLED lib with the version from https://github.com/ben-xo/FastLED . My version has modifications to enable interrupts in certain places in order to not drop samples.
4) Make sure the modified FastLED is checked out to the branch `feature/ben-xo-integration-branch`
5) Load vu3.ino into Arduino Studio
6) edit config.h . set your strip length. maybe comment out the DEBUG defines if i left them uncommented
7) program the arduino over USB. (or use ./build-and-upload.sh if you went with arduino-cli)

*N.B* You may want to upgrade avr-gcc to version 11. Setting this up is out of scope of the README. 

Rough instructions for that:
* `brew install arduino-cli`
* `brew install avr-gcc@11`
* `brew link avr-gcc@11`
* modify `~/Library/Arduino15/packages/arduino/hardware/avr/1.8.3/platform.txt` - change `compiler.path` to `/usr/local/bin/`
* use `./build-and-upload.sh` instead of Arduio Studio to program the chip

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

This is "what can I squeeze out of an Arduino" project the architecture is quite Arduino specific and heavily timer interrupt driven.

The main loop does does the following:
* processes all samples queued up in the sample buffer to calculate a rolling VU, DC offset and beat detection filter
* stores sample-accurate "beat" information, and recalculates the estimates tempo (which looks a lot better for drum & bass than just the kick detection on its own)
* renders a frame of visualiser
* outputs the LEDs using FastLED

Meanwhile, there are very fast interrupt handlers for sampling the audio, doing PWM on the status LEDs, and maintaining an accurate FPS.


Want to work on it?
-------------------

Yeah! You do! I have no idea what to tell you, ask me questions on Twitter @benxo I have loads of ideas and none of them are important.

Thanks for coming to my TED talk


History
=======

* Worked on it throughout 2017 to 2019
* Made it public in a fit of GitHub and Twitter dopamine-seeking on 11th June 2020
* Made it all-in-one with no second arduino required for beat detect and very fast 180fps Jan 2021


Thanks
======

Have fun and no you can't use it commercially



