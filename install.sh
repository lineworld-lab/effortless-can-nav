#!/bin/bash

sudo apt install -y linux-headers-$(uname -r) build-essential libtool automake tree dkms

git clone https://gitlab.com/etherlab.org/ethercat.git

cd ethercat

git switch -c stable-1.6 origin/stable-1.6

./bootstrap 

./configure --enable-generic --disable-8139too --disable-e100 --disable-e1000 --disable-e1000e --disable-igb --disable-r8169 --disable-ccat --enable-static=no --enable-shared=yes --enable-eoe=no --enable-cycles=no --enable-hrtimer=no --enable-regalias=no --enable-tool=yes --enable-userlib=yes --enable-sii-assign=yes --enable-rt-syslog=yes

./make all modules 