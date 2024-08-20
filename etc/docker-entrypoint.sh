#!/bin/bash
set -o errexit
set -o nounset

PREFIX=""
DEFAULT_MODE="normal"

OBS_INSTALL_PATH=${OBS_INSTALL_PATH:-"/opt/obs-studio-portable"}
OBS_HEADLESS_INSTALL_PATH=${OBS_HEADLESS_INSTALL_PATH:-"/opt/obs-headless/"}

################################################################################

if [ "$MODE" = 'gdb' ]; then
	PREFIX="gdb --args"

elif [ "$MODE" = 'memcheck' ]; then
	supp=""
	#supp="--gen-suppressions=all --log-file=/root/etc/memcheck.log"
	#supp="$supp --suppressions=/root/etc/memcheck_suppressions.txt"

	mode="--tool=memcheck --trace-children=yes --leak-check=full \
			--leak-resolution=high --show-reachable=yes --track-fds=yes \
			--track-origins=yes --num-callers=32 --show-leak-kinds=all \
			--error-limit=no $supp"

	PREFIX="valgrind $mode"

elif [ "$MODE" = 'callgrind' ]; then
	PREFIX="valgrind --tool=callgrind --callgrind-out-file=/tmp/obs-headless.callgrind"
fi

################################################################################

# Run from OBS's directory. Core dumps will be located there.
echo "OBS_INSTALL_PATH: $OBS_INSTALL_PATH"
cd ${OBS_INSTALL_PATH}/bin/64bit/

echo "PREFIX: $PREFIX"

exec $PREFIX ${OBS_HEADLESS_INSTALL_PATH}/obs_headless_server