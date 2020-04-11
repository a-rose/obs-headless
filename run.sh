#!/bin/bash

. config.sh

here=$(pwd)
cd ${OBS_INSTALL_PATH}/bin/64bit/
${here}/build/obs_headless "$@"