What am I?
==========

* I am a vanity project (that I've been working, on and off, for several years)
* I make pretty sound reactive lights from line-in audio on a single Arduino with minimal extra components

Features
========

All of the following on a single Arduino:

* Interrupt driven audio sampling - even with WS2812s (requires a slightly modified FastLED library at https://github.com/ben-xo/FastLED) - straight from a line-in
* A solid 180 frame per second with 60 WS2812s (or 125fps with 100 WS2812s) with no slow frames or dropped samples
* Beat detection and tempo estimation in realtime. Yep! All on a single Arduino
* 8 cool sound-reactive visualisers. These have been endlessly tinkered with so that they look okay at a range of input volumes and aren't TOO jarring when they change
* Automatic mode changing after a certain number of beats
* Effects work well for a range of music from house and techno to drum & bass
* Visualisers cope with things like DC Offset on the line-in
* push button to change modes
* Status LEDs driven using interrupt driven PWM (so no need for resistors on the status LEDs)
* Screensaver when the audio goes quiet. Because why not!

Architecture
============

This is "what can I squeeze out of an Arduino" project the architecture is quite Arduino specific and heavily timer interrupt driven.

The main loop does does the following:
* processes all samples queued up in the sample buffer to calculate a rolling VU, DC offset and beat detection filter
* stores sample-accurate "beat" information, and recalculates the estimates tempo (which looks a lot better for drum & bass than just the kick detection on its own)
* renders a frame of visualiser
* outputs the LEDs using FastLED

Meanwhile, there are very fast interrupt handlers for sampling the audio, doing PWM on the status LEDs, and maintaining an accurate FPS.

Description of the Hardware
===========================

I refuse to draw an ASCII diagram so you'll have to interpret my words.

Audio Input
-----------

The audio input stage expects "0" to be at around 2.5v (i.e the mid range of the analog input).

Minimum would be something like:
* ground of audio connected to ground of arduino
* signal (left or right audio channel) connected to A0 through a 3.3uF electrolytic cap
* signal connected to ground via a 103 cap
* signal connected to ground and +5v through 100k resistors (to create a potential divider to get the signal to sit at 2.5v)

That's it.

Output LEDs
-----------

* connect 5 LEDs from D8, D9, D10, D11 and D12 to ground. (no resistor needed). These show the current mode in binary
* connect 2 LEDs from D3 and D2 to ground via 220 resistor. These flash in time for the beats.

Mode Button
-----------

* Pull D4 to ground through a 10k resistor (if you don't and it floats the mode will go haywire)
* Connect a push button from D4 to pull up to 5v. That's it

LED light strip
---------------

Connect some WS2812s or whatever through a 330 resistor to D6, in "the usual way".


Debug monitoring
----------------

If you wanna watch the frame rate (and time taken to do various stages in the main loop) on an oscilloscope, you should pull A1 and A2 to ground via 10k resistors and then hook your scope up to those pins.


Thanks
======

Have fun and no you can't use it commercially

- Ben
