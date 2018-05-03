# obs-headless
Headless obs test using libobs

## QT dependcy
The test program depends on QT5 because libobs doesn't seem to initialize OpenGL properly (segfault in gl_context_create when calling obs_reset_video).
Calling the QT init function beforehand seems to bypass this issue.

This could be a bug in libobs, the main obs frontend is not affected because it uses QT.

## Building
First, install obs from source in the obs-headless/ directory (for instructions, see https://github.com/obsproject/obs-studio/wiki/Install-Instructions ).

Then use ./compile.sh

## Usage
After compiling, use ./build/obs_headless -h