#!/bin/bash

rm -r build_jetson

mkdir -p build_jetson

cd build_jetson

cmake -DBUILD_JETSON=y ..

make

