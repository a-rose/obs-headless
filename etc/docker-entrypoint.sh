#!/bin/bash
set -o errexit
set -o nounset

OBS_INSTALL_PATH=${OBS_INSTALL_PATH:-"/opt/obs-studio-portable"}
OBS_HEADLESS_INSTALL_PATH=${OBS_HEADLESS_INSTALL_PATH:-"/opt/obs-headless/"}

# Run from OBS's directory. Core dumps will be located there.
echo "OBS_INSTALL_PATH: $OBS_INSTALL_PATH"
cd ${OBS_INSTALL_PATH}/bin/64bit/
${OBS_HEADLESS_INSTALL_PATH}/obs_headless_server