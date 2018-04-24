#!/bin/bash

PROGRAM_NAME="obs_test"
SRC="main.cpp"
CPP_FLAGS="-std=gnu++0x -DDL_OPENGL=\"libobs-opengl\""
INCLUDE_PATH="-Iobs-studio/libobs/"
LIBRARIES="-lobs"
LIBRARIES_PATH="-Lobs-studio/build/libobs/"

g++ ${CPP_FLAGS} ${INCLUDE_PATH} ${SRC} ${LIBRARIES} ${LIBRARIES_PATH} -o ${PROGRAM_NAME}
chmod +x ${PROGRAM_NAME}
echo "Done"