#!/bin/bash

mkdir -p build
cd build
cmake ..
make -j4
echo -e "\n\n\033[32mDone, start with ./build/obs_headless\033[0m"