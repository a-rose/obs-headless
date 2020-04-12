#pragma once
#include <string>

using namespace std;

struct Settings {
    string stream_a;
    string stream_b;
    string server;
    string key;
    int transition_delay_sec;
    int transition_duration_ms;
    int video_bitrate_kbps;
    int video_width;
    int video_height;
    int video_fps_num;
    int video_fps_den;
};

Settings LoadConfig(const string& file);