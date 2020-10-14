Toyota 2006 Yaris Steering Wheel Controls to Kenwood NEC Remote Protocol Adaptor
================================================================================

Why spend £30 on an adaptor when you can spend hours on DIY instead (and potentially learn something)?

I'm using this with an ATtiny13a in the circuit illustrated by this snazzy and extremely legible diagram:

![Circuit diagram within car](./circuit_diagram.png)

The steering wheel buttons introduce resistors between the pink/purple wires and the black wire. So, the black wire is grounded, and we use the internal pull-ups on the GPIO pins to get one analogue and one digital input. The remote wire is pulled up to 3v3 by the radio, so that pin starts tri-stated, and is intermittently grounded to transmit the remote protocol's pulses.

The volume and prev/next (up/down) buttons do their usual thing.

The MODE button has three functions:

- Press once for mute/unmute (useful to quickly deal with annoying audio ads, or passenger phone calls).

- Double-press for play/pause. (On the second press, the first press's mute will be unmuted before play/pause is sent.)

- Hold — and then repeatedly press — for source switching.

The code is barely optimized; just enough to make it fit in the 1kB available. To modify it with more functions, you might need to improve the code layout, or just buy an ATtiny with more flash memory!

Your individual microcontroller and power supply might see different analogue values from the switches' resistors. See `measurement2` branch for program to read out values with the LED.

An earlier version of this project used an (expensive) Arduino Micro in place of the ATtiny. See other branch for (out-of-date) Arduino code.

This has been tested with a UK Yaris II from early 2006, and a Kenwood KDC-BT35U radio.


Useful sources
--------------

- [SB-Projects' explanation of NEC protocol](https://www.sbprojects.net/knowledge/ir/nec.php) (2001)
- [My previous Raspberry Pi "NAD Link" project (also NEC protocol)](https://github.com/tsprlng/nad-link) (~2015)
- [Matti Kantola's equivalent project for Renault Megane](http://www.angelfire.com/nd/maza/kenwood.html) (2000)


Here, for archival purposes, are all the Kenwood remote codes from Matti Kantola's page, which I could not find elsewhere!

Without these, it would have been an impossible nightmare.

| meaning    | code   |
|------------|--------|
| tuner      | 0xb91c |
| tape       | 0xb91d |
| cd         | 0xb91e |
| cd-md-ch   | 0xb91f |
| track-     | 0xb90a |
| track+     | 0xb90b |
| rew        | 0xb90c |
| ff         | 0xb90d |
| dnpp (?)   | 0xb95e |
| play/pause | 0xb90e |
| ---        |        |
| volume+    | 0xb914 |
| volume-    | 0xb915 |
| source     | 0xb913 |
| mute       | 0xb916 |
| ---        |        |
| 0          | 0xb900 |
| 1          | 0xb901 |
| 2          | 0xb902 |
| 3          | 0xb903 |
| 4          | 0xb904 |
| 5          | 0xb905 |
| 6          | 0xb906 |
| 7          | 0xb907 |
| 8          | 0xb908 |
| 9          | 0xb909 |
