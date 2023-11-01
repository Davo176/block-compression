#!/bin/bash

Xvfb :0 -screen 0 1024x768x16 &
export DISPLAY=:0.0
python3 linux_runner.py cRLE.exe the_combinatorial_one_42000000_14x10x12.csv -v
