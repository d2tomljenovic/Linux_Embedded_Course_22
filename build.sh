#!/bin/bash
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
echo "Make clean executed"
make clean
echo "Building"
make
