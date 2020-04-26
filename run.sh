#!/bin/bash

. config.sh

here=$(pwd)
cd ${OBS_INSTALL_PATH}/bin/64bit/

if [ "$1" == "-g" ]; then
    gdb ${here}/build/obs_headless
else
    ${here}/build/obs_headless
fi