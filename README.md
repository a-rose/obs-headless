# obs-headless

C++ program based on libobs (from obs-studio), designed to read RTMP streams and compose them as sources in different scenes.

The main part consists of a gRPC server. An example client is also provided.

This project uses Docker to ease build and deployment. If you follow the prerequisites below, accessing the GPU with Docker should work out of the box.

# TLDR: let's stream!

	cp .env.dist .env

Edit `.env` and `etc/config.txt` as needed.

Generate video test sources, build and run the server:

	make generate build up

Start the client in an other terminal:

	make client

Play the output stream in another terminal:

	make play

# Architecture overview

## Services

[Services and tools used to run the project](architecture.png)

- `server` is a stripped down OBS: it reads input sources and streams its output to the desired endpoint.
- `client` controls the server remotely. Its main role is to load a "show" file (etc/shows/default.json) and send commands to the server to make it create and manage the sources and scenes required to compose the show.
	- For example, when you run the client, you'll be able to switch from one source to another by pressing a key.
	- The provided client is meant as an example implementation. I was reworking the project to extract the client from the server code, but this is paused due to my GPU problems.

The services below are placeholders for development, you might not need them if you want to use other sources and output endpoint.

- `rtsp` is used as an input media server (relaying the input test sources) and as an output media server (receiving obs-server-headless' output).
- `sourceA` and `sourceB` stream video test sources to act as live content.
Screenshot 2024-11-06 at 11 22 52

## Config

The input sources are set the default show in `etc/shows/default.json`.
The output endpoint is set as server + key in `etc/config.txt`.

**Input**: edit `etc/shows/default.json` to set the default scene when starting obs-headless. It contains two RTMP sources as inputs, for which you must set the URL of public or local RTMP streams (see STREAMING.md).

**Output**: edit `config.txt` to set `server` and `key` with your output stream URL and key. You can stream to any platform supporting RTMP (Twitch, Youtube, ...). You can also use any local RTMP server (see STREAMING.md).


# Getting started

## Prerequisites

- `make`, `docker` and `docker compose` installed.
- X Server
- NVidia GPU, and NVidia drivers installed.
	- On Ubuntu: `sudo ubuntu-drivers install`. More info: https://ubuntu.com/server/docs/nvidia-drivers-installation
	- Ensure `nvidia-smi` works. If not, you might need to disable Secure Boot in your BIOS.
- Docker + Nvidia tutorial: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html

### CUDA versions

Check which CUDA version is installed on your host using `nvidia-smi`. If needed, edit `Dockerfile` to use the same version as a base image: `FROM nvidia/cudagl:<YOUR CUDA VERSION HERE>-devel-ubuntu24.04`

Existing tags: https://hub.docker.com/r/nvidia/cudagl/tags

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
