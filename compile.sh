#!/bin/bash

. config.sh                                                                                                                              

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_INSTALL_RPATH="${BUILD_DIR}/lib" \
    -DOBS_INSTALL_PATH="${OBS_INSTALL_PATH}"
make -j$(nproc) && echo -e "\n\n\033[32mDone, start with ./run.sh\033[0m"
