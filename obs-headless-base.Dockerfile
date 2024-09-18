FROM nvidia/cuda:12.6.0-devel-ubuntu24.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /usr/local/src

# Packages:
#	- libdatachannel dependencies
#	- from OBS build instructions (see 
# https://github.com/obsproject/obs-studio/wiki/build-instructions-for-linux)
#		- build system
#		- core dependencies
#		- UI dependencies
#		- plugin dependencies
#	- from cudagl Dockerfile
#	- OBS dependencies
#	- debug tools (removed in the release image)
RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		libssl-dev \
		\
		cmake ninja-build pkg-config clang clang-format build-essential curl \
		ccache git \
		\
		libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev \
		libavutil-dev libswresample-dev libswscale-dev libx264-dev \
		libcurl4-openssl-dev libmbedtls-dev libgl1-mesa-dev libjansson-dev \
		libluajit-5.1-dev python3-dev libx11-dev libxcb-randr0-dev \
		libxcb-shm0-dev libxcb-xinerama0-dev libxcb-composite0-dev \
		libxcomposite-dev libxinerama-dev libxcb1-dev libx11-xcb-dev \
		libxcb-xfixes0-dev swig libcmocka-dev libxss-dev libglvnd-dev \
		libgles2-mesa-dev libwayland-dev \
		libsrt-openssl-dev libpci-dev libpipewire-0.3-dev libqrcodegencpp-dev \
		uthash-dev librist-dev \
		\
		qt6-base-private-dev libqt6svg6-dev qt6-wayland \
		qt6-image-formats-plugins \
		\
		libasound2-dev libfdk-aac-dev libfontconfig-dev libfreetype6-dev \
		libjack-jackd2-dev libpulse-dev libspeexdsp-dev libudev-dev libv4l-dev \
		libva-dev libvlc-dev libvpl2 libvpl-dev libdrm-dev nlohmann-json3-dev \
		libwebsocketpp-dev libasio-dev \
		\
		vim gdb valgrind net-tools iptables procps tcpdump \
		linux-tools-common linux-tools-generic

# Install FFnvcodec (OBS dependency)
RUN git clone https://git.videolan.org/git/ffmpeg/nv-codec-headers.git \
	&& cd nv-codec-headers \
	&& git checkout n12.1.14.0 \
	&& make -j $(nproc) \
	&& make install

# Install libdatachannel (OBS dependency)
RUN git clone https://github.com/paullouisageneau/libdatachannel.git \
	&& cd libdatachannel \
	&& git checkout v0.20.3 \
	&& git submodule update --init --recursive --depth 1 \
	&& cmake -B build -DUSE_GNUTLS=0 -DUSE_NICE=0 -DCMAKE_BUILD_TYPE=Release \
	&& cd build \
	&& make -j $(nproc) \
	&& make install

# gRPC
RUN git clone --recurse-submodules -b v1.66.1 --depth 1 --shallow-submodules https://github.com/grpc/grpc
RUN cd grpc \
	&& mkdir -p cmake/build \
	&& cd cmake/build \
	&& cmake \
		-DgRPC_INSTALL=ON \
		-DgRPC_BUILD_TESTS=OFF \
		-DgRPC_SSL_PROVIDER=package \
		-DABSL_ENABLE_INSTALL=ON \
		-DBUILD_SHARED_LIBS=ON \
		../.. \
	&& make -j $(nproc) \
	&& make install
