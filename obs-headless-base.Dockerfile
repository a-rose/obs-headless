FROM nvidia/cuda:12.0.0-devel-ubuntu22.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /usr/local/src

# Packages:
#	- librist dependencies
#	- from OBS build instructions:
#		- build system
#		- core dependencies
#		- UI dependencies
#		- plugin dependencies
#	- from cudagl Dockerfile
#	- OBS dependencies
#	- gRPC
#	- debug tools (removed in the release image)
RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		meson \
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
		libgles2-mesa libgles2-mesa-dev libwayland-dev \
		libsrt-openssl-dev libpci-dev \
		\
		qtbase5-dev qtbase5-private-dev libqt5svg5-dev qtwayland5 \
		libqt5x11extras5-dev \
		\
		libasound2-dev libfdk-aac-dev libfontconfig-dev libfreetype6-dev \
		libjack-jackd2-dev libpulse-dev libsndio-dev libspeexdsp-dev \
		libudev-dev libv4l-dev libva-dev libvlc-dev libdrm-dev \
		\
		libgrpc++-dev libgrpc++1 libgrpc-dev libgrpc10 \
		libprotobuf-dev protobuf-compiler-grpc \
		\
		vim gdb valgrind net-tools iptables procps tcpdump \
		linux-tools-common linux-tools-generic

# Install librist (OBS dependency)
RUN git clone https://code.videolan.org/rist/librist.git \
	&& cd librist \
	&& git checkout v0.2.7 \
	&& mkdir build \
	&& cd build \
	&& meson .. \
	&& ninja \
	&& ninja install
