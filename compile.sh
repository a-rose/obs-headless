#!/bin/bash

. config.sh                                                                                                                              

echo -e "\033[32mGenerating proto files...\033[0m"
cd proto/
sh gen_proto.sh
cd ..

echo -e "\033[32mPreparing build...\033[0m"
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DCMAKE_INSTALL_RPATH="${BUILD_DIR}/lib" \
    -DOBS_INSTALL_PATH="${OBS_INSTALL_PATH}"

echo -e "\033[32mBuilding...\033[0m"
make -j$(nproc) && echo -e "\n\033[32mDone.\nStart the server with ./run_server.sh\nStart the client with ./build/obs_headless_client\033[0m"
