FROM nvidia/cuda:12.0.0-devel-ubuntu20.04 as builder

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /usr/local/src

# From cudagl Dockerfiles
RUN apt-get update && apt-get install -y --no-install-recommends \
		pkg-config libglvnd-dev libgl1-mesa-dev \
		libegl1-mesa-dev libgles2-mesa-dev && \
	rm -rf /var/lib/apt/lists/*

# Dependencies
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

#------------------------------------------------------------------------------

# Get OBS sources
RUN git clone --recursive https://github.com/obsproject/obs-studio.git \
	&& cd obs-studio \
	&& mkdir build

# Checkout a specific OBS version
ENV OBS_VERSION=23.2.1
RUN cd obs-studio \
	&& git checkout ${OBS_VERSION}

# Build OBS
ENV OBS_INSTALL_PATH="/opt/obs-studio-portable"
RUN cd obs-studio/build \
	&& cmake .. \
		-DCMAKE_INSTALL_PREFIX=${OBS_INSTALL_PATH} \
		-DUNIX_STRUCTURE=0 \
		-DBUILD_BROWSER=OFF \
		-DENABLE_PIPEWIRE=OFF \
	&& make -j$(nproc) \
	&& make install

#------------------------------------------------------------------------------

# obs-headless
ENV OBS_HEADLESS_INSTALL_PATH="/opt/obs-headless"
COPY src/ /usr/local/src/obs-headless
ARG BUILD_TYPE=Release
RUN cd obs-headless \
	\
	&& echo -e "\033[32mBUILD_TYPE: ${BUILD_TYPE}\033[0m" \
	&& echo -e "\033[32mGenerating proto files...\033[0m" \
	&& cd proto_gen/ \
	&& sh proto_gen.sh \
	&& cd .. \
	\
	&& echo -e "\033[32mPreparing build...\033[0m" \
	&& mkdir -p build \
	&& cd build \
	&& cmake .. \
		-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
		-DCMAKE_INSTALL_PREFIX="${OBS_HEADLESS_INSTALL_PATH}" \
		-DCMAKE_INSTALL_RPATH="${OBS_HEADLESS_INSTALL_PATH}/lib" \
		-DOBS_INSTALL_PATH="${OBS_INSTALL_PATH}" \
	\
	&& echo -e "\033[32mBuilding...\033[0m" \
	&& make -j$(nproc) \
	&& make install \
	\
	&& if [ "${BUILD_TYPE}" = "Debug" ]; then \
		echo -e "\033[32mDebug mode: installing tools...\033[0m" \
		&& apt-get update \
		&& apt-get install -y --no-install-recommends \
			vim gdb valgrind net-tools iptables procps tcpdump \
			linux-tools-common linux-tools-generic; \
	fi


COPY etc/ /opt/obs-headless/etc

#------------------------------------------------------------------------------

# Copy the whole filesystem in a new build stage, to flatten the layers into
# one and get a minimal image size
FROM scratch
COPY --from=builder / /

ENTRYPOINT ["/opt/obs-headless/etc/docker-entrypoint.sh"]