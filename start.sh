#!/usr/bin/env bash

echo "start VOR receiver"

./VORreceiver.py &
./Instrument &
