# obs-headless
C++ program based on libobs (from obs-studio), designed to read RTMP streams and compose them as sources in different scenes.
This implementation is a gRPC server with a test client .

⚠️ At the moment, obs-headless only **works with old versions of libobs**. Version 23.2.1 is used.

# Prerequisites

- Machine with an NVidia GPU and NVidia drivers installed.
- X Server
- Docker + Nvidia tutorial: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html
- ffmpeg for tests

## CUDA versions

Check which CUDA version is installed on your host using `nvidia-smi`. If needed, edit `Dockerfile` to use the same version as a base image: `FROM FROM nvidia/cudagl:<YOUR CUDA VERSION HERE>-devel-ubuntu20.04`

Existing tags: https://hub.docker.com/r/nvidia/cudagl/tags

# Building

	./docker.sh build dev

# Running

## X Server Access Control

In order to allow the container to use the host's X Server, the `docker.sh` script runs the `xhost +` command everytime you use the `run`, `shell` or `gdb` actions.

You can undo this by executing `xhost -` on your host machine.

## Streaming Sources

`etc/shows/default.json` is used as a default scene when starting obs-headless. It contains two RTMP sources as inputs; they can be publicly available RTMP streams (for example `rtmp://213.152.6.234/iltv/high` or `rtmp://62.113.210.250/medienasa-live/ok-merseburg_high`) or streams produced locally using ffmpeg:

**Stream a local FLV file on port 1936:**

	ffmpeg -stream_loop -1 -re -i myfile.flv  -c copy -f flv  rtmp://localhost:1936/myfile


**Transcode and stream local file on port 1936:**

	ffmpeg -stream_loop -1 -re -i myfile.mp4 -c:v libx264 -b:v 2M -maxrate 2M -bufsize 1M -g 60 -c:a aac -b:a 128k -f flv rtmp://0.0.0.0:1936/

## Streaming Output

You can use any streaming platform, like Twitch, or you can start a RTMP server on your machine using ffmpeg.

	ffmpeg -y -f flv -listen 1 -i rtmp://localhost:1938/live/key -c copy obs-headless-out.flv -loglevel debug

Edit etc/config.txt to set `server` and `key` with your stream URL and key.

## Starting obs-headless

	./docker.sh run dev

For debugging:

	./docker.sh gdb dev

---

Start the test **client** in an other terminal:

	./docker.sh client

From the client, you can switch using by pressing `s` and `Enter`.

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
- [*] various TODOs in the code
- [*] pointers to ref