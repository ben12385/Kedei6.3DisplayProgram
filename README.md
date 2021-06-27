# GivingItAGo

Program to mirror framebuffer to kedei6.3 3.5 inch screen. I realised that nobody wrote any form of program to replace the driver provided by kedei so I decided to do one up myself. Currently only can display, touchscreen not working. Got the idea to only redraw areas that have changes, idea taken from HackaDay. Redraw speed is faster if there are consecutive pixels of the same colour.

Reasons to do it:
Driver didnt work for all kernel, this program should work as long as spi is usable.
No form of documentation of the display, so try to create one through trial and error.

Build command
gcc -Wall -o kedeiDisplay ./cDisplay.c -lwiringPi

Thanks to:
saper-2\rpi-spi-lcd35-kedei for giving a basic framework to work with.
