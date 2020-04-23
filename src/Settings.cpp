#include <fstream>
#include <sstream>
#include <ios>
#include "Settings.hpp"

Settings LoadConfig(const string& file) {
    Settings s;
    ifstream config(file);
    string line;

    if(config.fail()) {
        throw invalid_argument("Cannot open config file:" + file);
    }

    while(getline(config, line)) {
        stringstream iss(line);
        string key;
        iss >> key;
        
        if(key == "server") {
            iss >> s.server;
        } else if(key == "key") {
            iss >> s.key;
        } 
        
        else if(key == " transition_type") {
            iss >> s.transition_type;
        } else if(key == "transition_delay_sec") {
            iss >> s.transition_delay_sec;
        } else if(key == "transition_duration_ms") {
            iss >> s.transition_duration_ms;
        }
        
        else if(key == " video_hw_encode") {
            iss >> s.video_hw_encode;
        } else if(key == " video_hw_decode") {
            iss >> s.video_hw_decode;
        } else if(key == " video_gpu_conversion") {
            iss >> s.video_gpu_conversion;
        } else if(key == "video_bitrate_kbps") {
            iss >> s.video_bitrate_kbps;
        } else if(key == " video_keyint_sec") {
            iss >> s.video_keyint_sec;
        } else if(key == " video_rate_control") {
            iss >> s.video_rate_control;
        } else if(key == "video_width") {
            iss >> s.video_width;
        } else if(key == "video_height") {
            iss >> s.video_height;
        } else if(key == "video_fps_num") {
            iss >> s.video_fps_num;
        } else if(key == "video_fps_den") {
            iss >> s.video_fps_den;
        }
        
        else if(key == " audio_sample_rate") {
            iss >> s.audio_sample_rate;
        } else if(key == " audio_bitrate_kbps") {
            iss >> s.audio_bitrate_kbps;
        } 
    }

    if(s.server == "") {
        throw invalid_argument("Cannot start without a server url.");
    }
    if(s.key == "") {
        throw invalid_argument("Cannot start without a stream key.");
    }
    if(s.transition_delay_sec < 0 || s.transition_delay_sec > 60*60*24) {
        throw invalid_argument("Invalid transition delay: " + to_string(s.transition_delay_sec));
    }
    if(s.transition_duration_ms < 0 || s.transition_duration_ms > 1000*60*60) {
        throw invalid_argument("Invalid transition duration: " + to_string(s.transition_duration_ms));
    }

    return s;
}
