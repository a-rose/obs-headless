#!/bin/bash
set -o errexit
set -o nounset

OBS_INSTALL_PATH=${OBS_INSTALL_PATH:-"/obs-studio-portable"}

# Run from OBS's directory. Core dumps will be located there.
echo "OBS_INSTALL_PATH: $OBS_INSTALL_PATH"
cd ${OBS_INSTALL_PATH}/bin/64bit/
/usr/local/src/obs-headless/build/obs_headless_server