#!/usr/bin/env bash

g++ -D_REENTRANT Instrument.cxx -o Instrument -lSDL2 -lSDL2_image -lSDL2_ttf -lpthread
gcc rotary.c -o gpiod -lwiringPi

