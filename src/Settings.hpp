#pragma once
#include <string>

using namespace std;

struct Settings {
    string server;
    string key;

    string transition_type;
    int transition_delay_sec;
    int transition_duration_ms;
    
    bool video_hw_decode;
    bool video_hw_encode;
    bool video_gpu_conversion;
    int video_bitrate_kbps;
    int video_keyint_sec;
    string video_rate_control;
    int video_width;
    int video_height;
    int video_fps_num;
    int video_fps_den;

    int audio_sample_rate;
    int audio_bitrate_kbps;
};

Settings LoadConfig(const string& file);