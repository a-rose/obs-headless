#!/bin/bash

. config.sh

here=$(pwd)
cwd="${OBS_INSTALL_PATH}/bin/64bit/"
cd $cwd

echo "Current working directory (for core dumps etc): $cwd"

if [ "$1" == "-g" ]; then
    gdb ${here}/build/obs_headless_server
else
    ${here}/build/obs_headless_server
fi