#!/bin/bash

BUILD_DIR="build"
INSTALL_DIR=".."
BUILD_TYPE=Release

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_INSTALL_RPATH="${BUILD_DIR}/lib"
make -j4 && echo -e "\n\n\033[32mDone, start with ./build/obs_headless\033[0m"
