#!/bin/bash

rm -r build_deb64

mkdir -p build_deb64

cd build_deb64

cmake -DBUILD_DEB64=y ..

make

