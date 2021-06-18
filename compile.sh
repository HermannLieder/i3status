#!/bin/bash

autoreconf -fi
mkdir build
cd build
../configure --disable-sanitizers
make -j$(nproc)
sudo make install
