#!/usr/bin/env bash

echo "stop VOR receiver"

pkill -9 -f VORreceiver.py &
killall Instrument
