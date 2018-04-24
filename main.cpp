#include <stdio.h>
#include "obs.h"
#include "trace.h"


bool enum_source_cb(void* data, obs_source_t* src) {

    // Use :c:func:`obs_source_get_ref()` or
    // :c:func:`obs_source_get_weak_source()` if you want to retain a
    // reference after obs_enum_sources finishes.

    trace("src: %p", src);
    return true; // return true to continue enumeration, or false to end enumeration.
}

bool enum_outputs_cb(void* data, obs_output_t* src) {

    // Use :c:func:`obs_source_get_ref()` or
    // :c:func:`obs_source_get_weak_source()` if you want to retain a
    // reference after obs_enum_sources finishes.

    trace("output: %p", src);
    return true; // return true to continue enumeration, or false to end enumeration.
}

bool enum_encoders_cb(void* data, obs_encoder_t* src) {

    // Use :c:func:`obs_source_get_ref()` or
    // :c:func:`obs_source_get_weak_source()` if you want to retain a
    // reference after obs_enum_sources finishes.

    trace("output: %p", src);
    return true; // return true to continue enumeration, or false to end enumeration.
}



int main(int argc, char** argv) {
    trace("obs test:");

    if(!obs_startup("en-US", nullptr, nullptr) || !obs_initialized()) {
        trace_error("obs_startup failed");
        return -1;
    }

    trace("obs version: %d",  obs_get_version());

    struct obs_video_info ovi;
    struct obs_audio_info oai;
#if 1
    ovi.adapter         = 0;
    ovi.fps_num         = 60000;
    ovi.fps_den         = 1000;
    ovi.graphics_module = DL_OPENGL;
    ovi.output_format   = VIDEO_FORMAT_RGBA;
    ovi.base_width      = 1920;
    ovi.base_height     = 1080;
    ovi.output_width    = 1920;
    ovi.output_height   = 1080;

    if (obs_reset_video(&ovi) != 0) {
        trace_error("Couldn't initialize video");
    }


    oai.samples_per_sec  = 48000;
    oai.speakers         = SPEAKERS_STEREO;

    if (obs_reset_audio(&oai) != 0) {
        trace_error("Couldn't initialize audio");
    }
#endif

    if(!obs_get_video_info(&ovi)) {
        trace_error("obs_get_video_info failed");
    }

    if(!obs_get_audio_info(&oai)) {
        trace_error("obs_get_audio_info failed");
    }


    obs_enum_sources(&enum_source_cb, nullptr);
    obs_enum_outputs(&enum_outputs_cb, nullptr);
    obs_enum_encoders(&enum_encoders_cb, nullptr);


    obs_shutdown();

    trace("exiting");
    return 0;
}