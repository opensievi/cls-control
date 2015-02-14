# cls-control

This program started as a simple Arduino-based Central Locking System control for my 1991 Mercedes 230TE (124.083) but since the host vehicle has been standing in my garage due to ongoing Megasquirt-EFI (later referred to as MS) conversion, I started to add functions and probably will keep on doing so infinitely.

Also this is my first GitHub-repository so that's another learning process for me.

# Hardware used

I use a Arduino Nano -clone (later referred to as Nano) that controls a 16x2 backlit LCD-display via an I2C-module.
For remote control I have a cheap chinese 4-channel remote control module that drives four SPDT-relays.
Temperature is told by two 1-wire DS18B20-sensors.
I also have a DS1307 I2C real time clock module to tell the time (forgot to add that to version history earlier, added now to current version).

Currently the hardware is assembled on a breadboard and leds simulate the outputs.

# External libraries

- Wire
- RTClib
- LiquidCrystal_I2C
- OneWire
- DallasTemperature


# To-Do

- The LCD-display backlight dimming is done by using an analog output from Nano to drive the backlight led. Propably not the best way to do it, will get to it later.
- The messages shown in the LCD-display are in finnish. I will add english comments later so they can be easily translated as one wishes.
- There is a welcome message when power is turned on saying "Starduino" and version number below that.
Starduino was the first name of this project but it was already used by someone else.
I'll have to think something else for the welcome message in near(ish) future.
- Bug squishing...


# Version history

## v0.3

- Ignition switch activates the LCD-diplay backlight.
- Added a DS1307 I2C RTC-module to tell the time.

## v0.2

- Added temperature display both inside and outside of the vehicle.
- Added option to use two remote buttons to control the cls.

## v0.1

- Remote controlled central locking system (CLS).


# Future visions

Some wild ideas that are not (yet) seriously planned but never say never...

- Adding an accelerometer + connection to MS via CAN.
- GPS support and datalogging from MS.
- Automatic climate controlling instead of manual air conditioning.
