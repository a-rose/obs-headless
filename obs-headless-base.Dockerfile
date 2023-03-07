FROM nvidia/cuda:12.0.0-devel-ubuntu20.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /usr/local/src

# Dependencies:
#	- basic tools
#	- from cudagl Dockerfile
#	- from OBS build instructions
#	- gRPC
#	- debug tools (removed in the release image)
RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		build-essential cmake git \
		\
		pkg-config libglvnd-dev libgl1-mesa-dev \
		libegl1-mesa-dev libgles2-mesa-dev \
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
		\
		libgrpc++-dev libgrpc++1 libgrpc-dev libgrpc6 \
		libprotobuf-dev protobuf-compiler-grpc \
		\
		vim gdb valgrind net-tools iptables procps tcpdump \
		linux-tools-common linux-tools-generic
