# obs-headless

C++ program based on libobs (from obs-studio), designed to read RTMP streams and compose them as sources in different scenes.

The main part consists of a gRPC server. An example client is also provided.

This project uses Docker to ease build and deployment. If you follow the prerequisites below, accessing the GPU with Docker should work out of the box.

⚠️ At the moment, obs-headless only **works with old versions of libobs**. Version 23.2.1 is used.

# Prerequisites

- Machine with an NVidia GPU and NVidia drivers installed.
- X Server
- `apt install make`
- Docker + Nvidia tutorial: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html
- Video sources and sinks to test with. Please read STREAMING.md for details on how to generate test streams.

## CUDA versions

Check which CUDA version is installed on your host using `nvidia-smi`. If needed, edit `Dockerfile` to use the same version as a base image: `FROM nvidia/cudagl:<YOUR CUDA VERSION HERE>-devel-ubuntu22.04`

Existing tags: https://hub.docker.com/r/nvidia/cudagl/tags

# Building and running

Build and run the server:

	make release
	make run

Start the client in an other terminal:

	make client

From the client, you can switch the source using by pressing `s` and `Enter`.

## X Server Access Control

In order to allow the container to use the host's X Server, the Makefile runs the `xhost +` command everytime you use the `run` or targets.

You can undo this by executing `xhost -` on your host machine.

## OBS version

You can specify which OBS version to build and run with, for example:

	make obs_version=26.1.2 release
	make obs_version=26.1.2 run

## Configuration

**Input**: edit `etc/shows/default.json` to set the default scene when starting obs-headless. It contains two RTMP sources as inputs, for which you must set the URL of public or local RTMP streams (see STREAMING.md).

**Output**: edit `config.txt` to set `server` and `key` with your output stream URL and key. You can stream to any platform supporting RTMP (Twitch, Youtube, ...). You can also use any local RTMP server (see STREAMING.md).

## Development



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

Using the base image: you can start a container with obs-studio and obs-headless sources attached as volumes, so you can edit sources and rebuild in the container.

1. Clone obs-studio on your host (see obs-headless-builder.Dockerfile for the repo URL)
2. Set `obs_sources` in Makefile to the path where you just cloned obs-studio
3. Start the container: `make base`.
4. Build obs-studio and obs-headless (see Dockerfiles for build instructions)
5. You can now edit the sources and rebuild from the container. Rebuild with `rb` and start with `st` (see etc/bashrc for aliases).

# TODO

- [fix] Playback stops when switching source!
- [fix] green screen when using OBS version > 23.2.1. At the moment, using (for example) v24.0.0 gives a green video output (audio is fine)
- [build] Fix runtime path; currently we need to cd into obs's install path (see run.sh) for obs to find the *.effect files in `find_libobs_data_file()`
- [build] CMake: `set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wno-long-long -pedantic")`
- [config] Support more transitions: [cut/fade/swipe/slide/stinger/fade_to_color/luma_wipe]_transition
- [feat] rescue
- [deps] fdk-aac, x264 / ffmpeg. explain ffmpeg_nvenc
- [style] fix mixed snake_case and camelCase
- [feat] trace level and format from env
- [docs] copy docs from src
- [docs] mention evans for tests, with examples
- [docker] use docker-compose with ffmpeg RTMP servers in containers
- [client] q must stop the server
- [client] show usage in cli (e.g. 's' to switch sources)
- [*] various TODOs in the code
- [*] pointers to ref
- [*] switch to Golang
