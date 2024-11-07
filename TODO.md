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
