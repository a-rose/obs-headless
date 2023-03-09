FROM nvidia/cuda:12.0.0-devel-ubuntu20.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /usr/local/src

# Packages:
#	- basic tools
#	- from cudagl Dockerfile
#	- SRT dependencies
#	- OBS dependencies
#	- gRPC
#	- debug tools (removed in the release image)
RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		build-essential cmake meson ninja-build git pkg-config \
		\
		libglvnd-dev libgl1-mesa-dev \
		libegl1-mesa-dev libgles2-mesa-dev \
		\
		tclsh libssl-dev \
		\
		libmbedtls-dev libasound2-dev libavcodec-dev libavdevice-dev \
		libavfilter-dev libavformat-dev libavutil-dev libcurl4-openssl-dev \
		libfdk-aac-dev libfontconfig-dev libfreetype6-dev \
		libjack-jackd2-dev libjansson-dev libluajit-5.1-dev libpulse-dev \
		libqt5x11extras5-dev libspeexdsp-dev libswresample-dev libswscale-dev \
		libudev-dev libv4l-dev libvlc-dev libwayland-dev libx11-dev \
		libx264-dev libxcb-shm0-dev libxcb-xinerama0-dev libxcomposite-dev \
		libxinerama-dev python3-dev qtbase5-dev qtbase5-private-dev \
		libqt5svg5-dev swig libxcb-randr0-dev libxcb-xfixes0-dev \
		libx11-xcb-dev libxcb1-dev libxss-dev qtwayland5 libgles2-mesa \
		clang clang-format curl ccache libcmocka-dev libpci-dev \
		libsndio-dev libva-dev libdrm-dev libxcb-composite0-dev \
		\
		libgrpc++-dev libgrpc++1 libgrpc-dev libgrpc6 \
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

# Install libsrt-openssl (OBS dependency)
RUN git clone https://github.com/Haivision/srt.git \
	&& cd srt \
	&& git checkout v1.5.1 \
	&& ./configure \
	&& make install -j$(nproc)
