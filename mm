#!/bin/bash
gcc -o msxslot -O3 -mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits msxbus.c -D_MAIN

