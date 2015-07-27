# Arduino dementia clock
This design is to help those dementia patients with reading the time, as an accompaniment to a normal clock to help with time recognition.
The readout loops through "NIGHT", "MORNING", "NOON", "AFTERNOON", "EVENING", "NIGHT" based on the hour of the day.
This is a very basic switch based on the fact I couldnt find anything on the market.
The LEDs are now out of production so will have to be replaced with a suitable display readout.

## Requirements
Requires Wire library with the clock set up for SDA/SCL pins as described here: https://www.arduino.cc/en/reference/wire

## Hardware
This setup uses:

* 1 * Arduino Micro
* 1 * Rotary encoder (with button): https://www.sparkfun.com/products/9117
* 1 * Real time clock: https://www.sparkfun.com/products/12708
* 9 * 16 segment LEDs (Now retired): https://www.sparkfun.com/products/retired/9933
* 18 * LED Switches to drive displays

Hardware diagram to be added later.
