FROM nvidia/cudagl:11.4.2-devel-ubuntu20.04

# Dependencies
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
		build-essential cmake git \
		\
        libmbedtls-dev libasound2-dev libavcodec-dev libavdevice-dev \
		libavfilter-dev libavformat-dev libavutil-dev libcurl4-openssl-dev \
		libfdk-aac-dev libfontconfig-dev libfreetype6-dev libglvnd-dev \
		libjack-jackd2-dev libjansson-dev libluajit-5.1-dev libpulse-dev \
		libqt5x11extras5-dev libspeexdsp-dev libswresample-dev libswscale-dev \
		libudev-dev libv4l-dev libvlc-dev libwayland-dev libx11-dev \
		libx264-dev libxcb-shm0-dev libxcb-xinerama0-dev libxcomposite-dev \
		libxinerama-dev pkg-config python3-dev qtbase5-dev qtbase5-private-dev \
		libqt5svg5-dev swig libxcb-randr0-dev libxcb-xfixes0-dev \
		libx11-xcb-dev libxcb1-dev libxss-dev qtwayland5 libgles2-mesa \
        libgles2-mesa-dev \
		\
		libgrpc++-dev libgrpc++1 libgrpc-dev libgrpc6 \
		libprotobuf-dev protobuf-compiler-grpc

# OBS
ENV OBS_VERSION=23.2.1
ENV OBS_INSTALL_PATH="${HOME}/obs-studio-portable"
RUN git clone --recursive https://github.com/obsproject/obs-studio.git \
	&& cd obs-studio \
	&& git checkout ${OBS_VERSION} \
	&& mkdir build \
	&& cd build \
	&& cmake -DUNIX_STRUCTURE=0 -DCMAKE_INSTALL_PREFIX=${OBS_INSTALL_PATH} -DBUILD_BROWSER=OFF .. \
	&& make -j$(nproc) \
	&& make install

# obs-headless
COPY . /usr/local/src/obs-headless
ARG BUILD_TYPE=Release
RUN cd /usr/local/src/obs-headless \
	\
	&& echo -e "\033[32mBUILD_TYPE: ${BUILD_TYPE}\033[0m" \
	&& echo -e "\033[32mGenerating proto files...\033[0m" \
	&& cd proto/ \
	&& sh gen_proto.sh \
	&& cd .. \
	\
	&& echo -e "\033[32mPreparing build...\033[0m" \
	&& mkdir -p build \
	&& cd build \
	&& cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
		-DCMAKE_INSTALL_PREFIX=".." -DCMAKE_INSTALL_RPATH="build/lib" \
		-DOBS_INSTALL_PATH="${OBS_INSTALL_PATH}" \
	\
	&& echo -e "\033[32mBuilding...\033[0m" \
	&& make -j$(nproc) \
	\
	&& if [ "${BUILD_TYPE}" = "Debug" ]; then \
		echo -e "\033[32mDebug mode: installing tools...\033[0m" \
		&& apt-get update \
    	&& apt-get install -y --no-install-recommends \
			vim gdb valgrind net-tools iptables procps tcpdump \
			linux-tools-common linux-tools-generic; \
	fi


# Run from OBS's directory. Core dumps will be located there.
WORKDIR ${OBS_INSTALL_PATH}/bin/64bit/
ENTRYPOINT ["/usr/local/src/obs-headless/build/obs_headless_server"]