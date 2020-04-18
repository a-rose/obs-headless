#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <mutex>
#include <thread>
#include "obs.h"
#include "ObsWorker.hpp"

//TODO
std::mutex obsapp_main_mutex;
//std::condition_variable obsapp_main_cv;
bool exit_worker_thread;


ObsWorker::ObsWorker(Settings settings_in)
    : settings(settings_in){
}

ObsWorker::~ObsWorker() {
    obs_shutdown();
}

int ObsWorker::LoadModule(const char* binPath, const char* dataPath) {
    obs_module_t *module;

    int code = obs_open_module(&module, binPath, dataPath);
    if (code != MODULE_SUCCESS) {
        trace_error("Failed to load module file '%s': %d", binPath, code);
        return -1;
    }

    if(obs_init_module(module) != true) {
        return -1;
    }

    return 0;
}

int ObsWorker::start() {
    trace("Starting thread...");
    exit_worker_thread = false;

    // Start the worker thread and get its thread ID
    std::thread worker_thread(do_work, settings);

    std::stringstream ss;
    ss << worker_thread.get_id();
    int video_thread_id = std::stoull(ss.str());

    // Wait for 'q' to stop the thread
    do {
        trace("Press 'q' key to stop. (video thread id: 0x%x)", video_thread_id);
    } while (cin.get() != 'q');

    trace("Stopping thread...");
    obsapp_main_mutex.lock();
    exit_worker_thread = true;
    obsapp_main_mutex.unlock();
    worker_thread.join();

    return 0;
}


void ObsWorker::do_work(Settings settings) {
    bool should_stop;
    int transition = 0;
    std::stringstream ss;

    struct obs_video_info ovi;
    struct obs_audio_info oai;

    obs_source_t*   rtmp_source_A;
    obs_source_t*   rtmp_source_B;
    obs_source_t*   transition_A;
    obs_source_t*   transition_B;
    obs_scene_t*    scene_A;
    obs_scene_t*    scene_B;
    obs_output_t*   output;
    obs_service_t*  service;
    obs_encoder_t*  enc_a;
    obs_encoder_t*  enc_v;

    obs_data_t*     rtmp_source_A_settings;
    obs_data_t*     rtmp_source_B_settings;
    obs_data_t*     enc_v_settings;
    obs_data_t*     rtmp_settings;

    obs_sceneitem_t *scene_item_A = NULL;
    obs_sceneitem_t *scene_item_B = NULL;


    try {

        if(!obs_startup("en-US", nullptr, nullptr) || !obs_initialized()) {
            throw string("obs_startup failed");
        }

        memset(&ovi, 0, sizeof(ovi));
        memset(&oai, 0, sizeof(oai));

        ovi.adapter         = 0;
        ovi.graphics_module = LIBOBS_PATH"libobs-opengl.so.0.0";
        ovi.output_format   = VIDEO_FORMAT_I420;
        ovi.fps_num         = settings.video_fps_num;
        ovi.fps_den         = settings.video_fps_den;
        ovi.base_width      = settings.video_width;
        ovi.base_height     = settings.video_height;
        ovi.output_width    = settings.video_width;
        ovi.output_height   = settings.video_height;

        if(obs_reset_video(&ovi) != OBS_VIDEO_SUCCESS) {
            throw string("obs_reset_video failed");
        }

        oai.samples_per_sec  = 44100;
        oai.speakers         = SPEAKERS_STEREO;

        if (obs_reset_audio(&oai) != true) {
            throw string("obs_reset_audio failed");
        }


        // Load modules
        if(0 != LoadModule(LIBOBS_PLUGINS_PATH "obs-ffmpeg.so", LIBOBS_PLUGINS_DATA_PATH "obs-ffmpeg")) {
            throw string("failed to load lib obs-ffmpeg.so");
        }

        if(0 != LoadModule(LIBOBS_PLUGINS_PATH "obs-transitions.so", LIBOBS_PLUGINS_DATA_PATH "obs-transitions")) {
            throw string("failed to load lib obs-transitions.so");
        }

        if(0 != LoadModule(LIBOBS_PLUGINS_PATH "rtmp-services.so", LIBOBS_PLUGINS_DATA_PATH "rtmp-services")) {
            throw string("failed to load lib rtmp-services.so");
        }

        if(0 != LoadModule(LIBOBS_PLUGINS_PATH "obs-x264.so", LIBOBS_PLUGINS_DATA_PATH "obs-x264")) {
            throw string("failed to load lib obs-x264.so");
        }

        // For rtmp-output
        if(0 != LoadModule(LIBOBS_PLUGINS_PATH "obs-outputs.so", LIBOBS_PLUGINS_DATA_PATH "obs-outputs")) {
            throw string("failed to load lib obs-outputs.so");
        }


        // stream A
        ss << "{ \"is_local_file\":false, \"input\":\"" << settings.stream_a <<"\", \"looping\":true }";
        trace("rtmp_source_A_settings: %s", ss.str().c_str());

        rtmp_source_A_settings = obs_data_create_from_json(ss.str().c_str());
        ss.str("");
        ss.clear(); // Clear state flags.

        if (!rtmp_source_A_settings) {
            throw string("Failed to initialize rtmp_source_A_settings");
        }

        rtmp_source_A = obs_source_create("ffmpeg_source", "ffmpeg source", rtmp_source_A_settings, nullptr);
        if (!rtmp_source_A) {
            throw string("Couldn't create rtmp_source_A source");
        }

        // Stream B
        ss << "{ \"is_local_file\":false, \"input\":\"" << settings.stream_b <<"\", \"looping\":true }";
        trace("rtmp_source_B_settings: %s", ss.str().c_str());
        rtmp_source_B_settings = obs_data_create_from_json(ss.str().c_str());
        ss.str("");
        ss.clear(); // Clear state flags.

        if (!rtmp_source_B_settings) {
            throw string("Failed to initialize rtmp_source_B_settings");
        }

        rtmp_source_B = obs_source_create("ffmpeg_source", "ffmpeg source", rtmp_source_B_settings, nullptr);
        if (!rtmp_source_B) {
            throw string("Couldn't create ffmpeg test source");
        }


        // transitions (contain sources)
        transition_A = obs_source_create("cut_transition", "cut transition", NULL, nullptr);
        if (!transition_A) {
            throw string("Couldn't create transition_A");
        }

        transition_B = obs_source_create("cut_transition", "cut transition", NULL, nullptr);
        if (!transition_B) {
            throw string("Couldn't create transition_B");
        }


        obs_transition_set(transition_A, rtmp_source_A);
        obs_transition_set(transition_B, rtmp_source_B);


        // scenes (contain transitions)
        scene_A = obs_scene_create("test scene");
        if (!scene_A) {
            throw string("Couldn't create scene_A");
        }
        scene_item_A = obs_scene_add(scene_A, transition_A);




        scene_B = obs_scene_create("test scene2");
        if (!scene_B) {
            throw string("Couldn't create scene_B");
        }
        scene_item_B = obs_scene_add(scene_B, transition_B);



        // output
        output = obs_output_create("rtmp_output", "rtmp output", NULL, nullptr);
        if (!output) {
            throw string("Couldn't create output");
        }

        // encoders
        enc_a = obs_audio_encoder_create("ffmpeg_aac", "aac enc", NULL, 0, nullptr);
        if (!enc_a) {
            throw string("Couldn't create enc_a");
        }

        enc_v = obs_video_encoder_create("obs_x264", "h264 enc", NULL, nullptr);
        if (!enc_v) {
            throw string("Couldn't create enc_v");
        }

        // video enc settings
        enc_v_settings = obs_encoder_get_settings(enc_v);
        if (enc_v_settings) {
            int bitrate = obs_data_get_int(enc_v_settings, "bitrate");
            trace_debug("video bitrate: %d", bitrate);

            obs_data_set_int(enc_v_settings, "bitrate", settings.video_bitrate_kbps);
            obs_encoder_update(enc_v, enc_v_settings);
            obs_data_release(enc_v_settings);
        } else {
            trace_error("Could not get video encoder settings");
        }

        // rtmp service
        ss << "{ \"server\":\"" << settings.server <<"\", \"key\":\"" << settings.key <<"\"}";
        trace("rtmp_settings: %s", ss.str().c_str());
        rtmp_settings = obs_data_create_from_json(ss.str().c_str());
        ss.str("");
        ss.clear(); // Clear state flags.

        if (!rtmp_settings) {
            throw string("Failed to initialize rtmp_settings");
        }
        service = obs_service_create("rtmp_common", "rtmp service", rtmp_settings, nullptr);
        if (!service) {
            throw string("Couldn't create service");
        }

        ///////////////////////////////////////////////////////////////////////////////


        obs_set_output_source(0, obs_scene_get_source(scene_A));
        obs_encoder_set_video(enc_v, obs_get_video());
        obs_encoder_set_audio(enc_a, obs_get_audio());
        obs_output_set_video_encoder(output, enc_v);
        obs_output_set_audio_encoder(output, enc_a, 0);
        obs_output_set_service(output, service);


        if(obs_output_start(output) != true) {
            throw string("obs_output_start failed");
        }

        while(should_stop == 0) {

            // todo cond wait
            sleep(1);
            obsapp_main_mutex.lock();
            should_stop = exit_worker_thread;
            obsapp_main_mutex.unlock();

            if(transition == settings.transition_delay_sec) {
                trace("Switch from stream A to stream B !");

                if(obs_transition_start(transition_A, OBS_TRANSITION_MODE_AUTO, settings.transition_duration_ms, transition_B) != true) {
                    throw string("obs_transition_start failed");
                }
            }

            transition++;
        }

        trace("Stopping thread...");

        obs_output_stop(output);
        obs_data_release(rtmp_source_A_settings);
        obs_data_release(rtmp_source_B_settings);
        obs_data_release(rtmp_settings);
        obs_encoder_release(enc_v);
        obs_encoder_release(enc_a);
        obs_service_release(service);
        obs_output_release(output);
        obs_source_release(rtmp_source_A);
        obs_source_release(rtmp_source_B);
        obs_source_release(transition_A);
        obs_source_release(transition_B);
        obs_sceneitem_release(scene_item_A);
        obs_sceneitem_release(scene_item_B);
        obs_scene_release(scene_A);
        obs_scene_release(scene_B);
    }
    catch(string e) {
        trace_error("An exception occured: %s", e.c_str());
    }
    catch(...) {
        trace_error("An uncaught exception occured !");
    }

    trace("exiting thread");
    return;
}
