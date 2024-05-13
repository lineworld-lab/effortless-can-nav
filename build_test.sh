#!/bin/bash

rm -r build_test

mkdir -p build_test

cd build_test

cmake -DBUILD_TEST=y ..

make

