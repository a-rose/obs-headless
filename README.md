# obs-headless
Headless obs test using libobs

## QT dependcy
The test program depends on QT5 because libobs doesn't seem to initialize OpenGL properly (segfault in gl_context_create when calling obs_reset_video).
Calling the QT init function beforehand seems to bypass this issue.

This could be a bug in libobs, the main obs frontend is not affected because it uses QT.

Using macOS, Qt is not needed, you can delete all references in the code and CMakeLists.txt.

## Getting Started

 - Build OBS from the sources. See https://github.com/obsproject/obs-studio/wiki/Install-Instructions#linux-portable-mode-all-distros .
 - Using the given cmake command-line, files are installed in `$HOME/obs-studio-portable` . If you change this path, you need to update `OBS_SOURCE_DIR` and `OBS_INSTALL_DIR` in `./compile.sh` to values relevant to your setup.
 - Run `./compile.sh`
 - After compiling, run `./build/obs_headless -h` for instructions

## TODO

- [ ] CMake: `set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror -Wno-long-long -pedantic")`
- [ ] HTML frontend example