# obs-headless

C++ program based on libobs (from obs-studio), designed to read RTMP streams and compose them as sources in different scenes.

The main part consists of a gRPC server. An example client is also provided.

This project uses Docker to ease build and deployment. If you follow the prerequisites below, accessing the GPU with Docker should work out of the box.


# Prerequisites

- `make`, `docker` and `docker compose` installed.
- X Server
- NVidia GPU, and NVidia drivers installed.
	- On Ubuntu: `sudo ubuntu-drivers install`. More info: https://ubuntu.com/server/docs/nvidia-drivers-installation
	- Ensure `nvidia-smi` works. If not, you might need to disable Secure Boot in your BIOS.
- Docker + Nvidia tutorial: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html

## CUDA versions

Check which CUDA version is installed on your host using `nvidia-smi`. If needed, edit `Dockerfile` to use the same version as a base image: `FROM nvidia/cudagl:<YOUR CUDA VERSION HERE>-devel-ubuntu24.04`

Existing tags: https://hub.docker.com/r/nvidia/cudagl/tags


# Getting started

## Video test sources

You can generate video sources to use as inputs (you only need to do this once):

	make generate

You can also use live sources, check out STREAMING.md for details.

## Building and running

Copy the distributed env file end edit it as needed:

	cp .env.dist .env


Build and run the server:

	make build
	make up

Start the client in an other terminal:

	make client

Play the output stream:

	make play

From the client, you can switch the source using by pressing `s` and `Enter`.

## X Server Access Control

In order to allow the container to use the host's X Server, the `xhost +` command is run when using `make up/make server/make dev`.

The effect of this command persists after running the container, you can undo this by executing `xhost -` on your host machine.

## Configuration

**Input**: edit `etc/shows/default.json` to set the default scene when starting obs-headless. It contains two RTMP sources as inputs, for which you must set the URL of public or local RTMP streams (see STREAMING.md).

**Output**: edit `config.txt` to set `server` and `key` with your output stream URL and key. You can stream to any platform supporting RTMP (Twitch, Youtube, ...). You can also use any local RTMP server (see STREAMING.md).


# Development

The build system uses three images:

- **obs-headless-base**
	- Dependencies only.
	- Can be used to experiment with different OBS versions, by mounting OBS and
		OBS-headless sources as a volume.
- **obs-headless-builder**:
	- Dependencies + OBS built from sources.
	- Can be used for development of OBS-headless, using a fixed version of OBS,
		by mounting sources as a volume.
- **obs-headless-dev**:
	- Dependencies + OBS + OBS-headless built in a single image.
	- Use this to run OBS-headless as a server.
- **obs-headless**:
	- Same as obs-headless-dev with an extra step to reduce the image size.
		Takes longer to build.

Using the dev image: you can start a container with obs-headless sources attached as volumes, so you can edit sources and rebuild in the container.

1. Start the test sources: `make testsrc`.
1. Start the dev container: `make dev`.
2. Build obs-headless (see Dockerfiles for build instructions)
3. You can now edit the code and rebuild from the container. Rebuild with `rb` and start with `st` (see etc/bashrc for aliases).

Using the base image, you can also build obs-studio from sources.

1. Clone obs-studio on your host (see obs-headless-builder.Dockerfile for the repo URL)
2. Set `OBS_SRC_PATH_DEV` in your .env file to the path where you just cloned obs-studio
3. Start the container: `make builder`.
4. Build obs-studio and obs-headless (see Dockerfiles for build instructions)
5. You can now edit the sources and rebuild from the container. Rebuild with `rb` and start with `st` (see etc/bashrc for aliases).

# TODO

- [build] update build system:
	```
	  ============ LEGACY BUILD SYSTEM IS DEPRECATED ============

	  You are using the legacy build system to build OBS Studio.  The legacy
	  build system is unsupported and will be removed in the near future.

	  To migrate to the new build system, familiarize yourself with CMake presets
	  (https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) and
	  create a user preset with your customized build settings, inheriting from
	  one of the default presets.

	  ============ LEGACY BUILD SYSTEM IS DEPRECATED ============
	```
- [fix] Playback stops when switching source!
- [build] Fix runtime path; currently we need to cd into obs's install path (see docker-entrypoint.sh) for obs to find the *.effect files in `find_libobs_data_file()`
- [build] CMake: `set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wno-long-long -pedantic")`
- [config] Support more transitions: [cut/fade/swipe/slide/stinger/fade_to_color/luma_wipe]_transition
- [feat] rescue
- [deps] fdk-aac, x264 / ffmpeg. explain ffmpeg_nvenc
- [style] fix mixed snake_case and camelCase
- [feat] trace level and format from env
- [docs] copy docs from src
- [docs] mention evans for tests, with examples
- [docker] reduce image size. use nvidia/cuda:12.0.0-runtime-ubuntu22.04 for release img
- [docker] Github Docker registry
- [client] show usage in cli (e.g. 's' to switch sources)
- [*] various TODOs in the code
- [*] pointers to ref
- [*] switch to Golang
- [server] segfault in libsrt when stopping (see https://github.com/Haivision/srt/issues/2770)
- [docker] move ldconfig in builder
