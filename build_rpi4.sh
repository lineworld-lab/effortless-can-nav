#!/bin/bash

rm -r build_rpi4

mkdir -p build_rpi4

cd build_rpi4

cmake -DBUILD_RPI4=y ..

make

