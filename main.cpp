#include "ObsWorker.hpp"
#include "trace.h"

using namespace std;

#define DEFAULT_STREAM_A "rtmp://184.72.239.149/vod/mp4:bigbuckbunny_450.mp4" // TODO
#define DEFAULT_STREAM_B "rtmp://str81.creacast.com:80/iltv/high"
#define DEFAULT_TRANSITION_DELAY_SEC 10
#define DEFAULT_TRANSITION_DURATION_MS 900



bool check_arg(const char* input, const char* option, const char* short_option) {
    return (    (option       && strcmp(input, option) == 0)
             || (short_option && strcmp(input, short_option) == 0) );
}

void print_help() {
    trace("\n"
        "--help, -h:                    this list.\n"
        "--stream_a <url>, -a:          address of the first stream to play (default: " DEFAULT_STREAM_A ").\n"
        "--stream_b <url>, -b:          address of the second stream to play (default: " DEFAULT_STREAM_B ").\n"
        "--stream_server, -s <url>:     server URL.\n"
        "--key, -k <string>:            stream private key.\n"
        "\n"
        "--transition_delay, -d <delay in seconds>:             delay before switching from stream A to stream B  (default: %d).\n"
        "--transition_duration, -t <duration in milliseconds>:  duration of the transition  (default: %d).\n",
        DEFAULT_TRANSITION_DELAY_SEC, DEFAULT_TRANSITION_DURATION_MS
    );
}


int main(int argc, char *argv[])
{
    worker_settings_t settings;
    settings.stream_a = DEFAULT_STREAM_A;
    settings.stream_b = DEFAULT_STREAM_B;
    settings.server = "";
    settings.key = "";
    settings.transition_delay_sec = DEFAULT_TRANSITION_DELAY_SEC;
    settings.transition_duration_ms = DEFAULT_TRANSITION_DURATION_MS;
    settings.video_bitrate_kbps = 800;
    settings.video_width = 640;
    settings.video_height = 360;
    settings.video_fps_num = 30000;
    settings.video_fps_den = 1000;

    char* end;

    if(argc < 2) {
        print_help();
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (check_arg(argv[i], "--help", "-h")) {
            print_help();
            return 0;
        }

        else if (check_arg(argv[i], "--stream_a", "-a")) {
            if (++i < argc) {
                settings.stream_a = string(argv[i]);
            }
        }
        else if (check_arg(argv[i], "--stream_b", "-b")) {
            if (++i < argc) {
                settings.stream_b = string(argv[i]);
            }
        }
        else if (check_arg(argv[i], "--stream_server", "-s")) {
            if (++i < argc) {
                settings.server = string(argv[i]);
            }
        }
        else if (check_arg(argv[i], "--stream_key", "-k")) {
            if (++i < argc) {
                settings.key = string(argv[i]);
            }
        }
        else if (check_arg(argv[i], "--transition_delay", "-d")) {
            if (++i < argc) {
                settings.transition_delay_sec = (int) strtol(argv[i], &end, 10);

                if (errno != 0 || end == argv[i]) {
                    trace_error("transition_delay: invalid string '%s'.", argv[i]);
                    return 1;
                }

            }
        }
        else if (check_arg(argv[i], "--transition_duration", "-t")) {
            if (++i < argc) {
                settings.transition_duration_ms = (int) strtol(argv[i], &end, 10);

                if (errno != 0 || end == argv[i]) {
                    trace_error("transition_duration: invalid string '%s'.", argv[i]);
                    return 1;
                }
            }
        }
    }

    if(settings.server == "") {
        trace_error("Cannot start without a server url.");
        return 1;
    }
    if(settings.key == "") {
        trace_error("Cannot start without a stream key.");
        return 1;
    }


    if(settings.transition_delay_sec < 0 || settings.transition_delay_sec > 60*60*24) {
        trace_error("Invalid transition delay: %d.", settings.transition_delay_sec);
        return 1;
    }

    if(settings.transition_duration_ms < 0 || settings.transition_duration_ms > 1000*60*60) {
        trace_error("Invalid transition duration: %d.", settings.transition_duration_ms);
        return 1;
    }


    trace("Start ObsWorker");
    try {
        ObsWorker worker(settings);
        worker.start();
    }
    catch(string e) {
        trace_error("An exception occured: %s", e.c_str());
    }
    catch(...) {
        trace_error("An uncaught exception occured !");
    }


    trace("Exit app");
    return 0;
}
