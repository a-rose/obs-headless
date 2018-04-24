#!/bin/bash

PROGRAM_NAME="obs_test"
SRC="main.cpp"
CPP_FLAGS="-std=gnu++0x -DDL_OPENGL=\"libobs-opengl\""
INCLUDE_PATH="obs-studio/libobs/"
LIBRARIES="-lobs"
LIBRARIES_PATH="-Lobs-studio/build/libobs/"

g++ ${CPP_FLAGS} -I${INCLUDE_PATH} ${SRC} ${LIBRARIES} ${LIBRARIES_PATH} -o ${PROGRAM_NAME}
chmod +x obs_test
echo "Done"