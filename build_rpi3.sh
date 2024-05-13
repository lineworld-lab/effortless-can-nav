#!/bin/bash

rm -r build_rpi3

mkdir -p build_rp3

cd build_rpi3

cmake -DBUILD_RPI3=y ..

make

