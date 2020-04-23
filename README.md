# obs-headless
C++ program based on libobs (from obs-studio), designed to read RTMP streams and compose them as sources in different scenes.

## QT dependcy
The test program depends on QT5 because libobs doesn't seem to initialize OpenGL properly (segfault in gl_context_create when calling obs_reset_video).
Calling the QT init function beforehand seems to bypass this issue.

This could be a bug in libobs, the main obs frontend is not affected because it uses QT.

Using macOS, Qt is not needed, you can delete all references in the code and CMakeLists.txt.

## Getting Started

Build OBS from the sources.
 You can follow [instructions from obs-studio on Github](https://github.com/obsproject/obs-studio/wiki/Install-Instructions#linux-portable-mode-all-distros) but watch out:

 - ⚠️ At the moment, obs-headless only **works with old versions of libobs**. Please use `git checkout 23.2.1` to use this old tag until this issue is resolved.
 - Using the given cmake command-line, files are installed in `$HOME/obs-studio-portable` . If you change this path, you need to update `OBS_INSTALL_PATH` in `./config.sh` to values relevant to your setup.

Now that OBS is installed, build and run obs-headless:
 - Run `./compile.sh`
 - After compiling, run `./run.sh -h` for instructions

## TODO

- [fix] green screen when using OBS version > 23.2.1. At the moment, using (for example) v24.0.0 gives a green video output (audio is fine)
- [fix] "double free" when exiting by pressing 'q'
- [build] Fix runtime path; currently we need to cd into obs's install path (see run.sh) for obs to find the *.effect files in `find_libobs_data_file()`
- [build] CMake: `set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wno-long-long -pedantic")`
- [test] Create interfaces like UDP or HTML server + frontend examples
- [test] create udp test script
- [config] Support more transitions: [cut/fade/swipe/slide/stinger/fade_to_color/luma_wipe]_transition
- [config] Duration type in ms
- [config] audio bitrate
- [config] video rate control
- [config] video GPU conversion
- [feat] rescue
- [feat] improve Trace
- [deps] fdk-aac, x264 / ffmpeg. explain ffmpeg_nvenc
- [feat] sigint handler in main for cleanup
- [style] fix mixed snake_case and camelCase