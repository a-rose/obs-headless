#include "ObsWorker.hpp"


//TODO
std::mutex obsapp_main_mutex;
//std::condition_variable obsapp_main_cv;
bool exit_worker_thread;


ObsWorker::ObsWorker(worker_settings_t settings_in)
    : settings(settings_in)
 {
    
}


ObsWorker::~ObsWorker() {
    obs_shutdown();
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


void ObsWorker::do_work(worker_settings_t settings) {

    bool should_stop;
    int transition = 0;
    std::stringstream ss;

    struct obs_video_info ovi;
    struct obs_audio_info oai;

    obs_source_t*   rtmp_source_A;
    obs_source_t*   rtmp_source_B;
    obs_source_t*   fade_transition_A;
    obs_source_t*   fade_transition_B;
    obs_scene_t*    scene_A;
    obs_scene_t*    scene_B;
    obs_output_t*   output;
    obs_service_t*  service;
    obs_encoder_t*  enc_a;
    obs_encoder_t*  enc_v;

    obs_data_t*     rtmp_source_A_settings;
    obs_data_t*     rtmp_source_B_settings;
    obs_data_t*     rtmp_settings;

    obs_sceneitem_t *scene_item_A = NULL;
    obs_sceneitem_t *scene_item_B = NULL;


    try {

        if(!obs_startup("en-US", nullptr, nullptr) || !obs_initialized()) {
            throw string("obs_startup failed");
        }

        ovi.adapter         = 0;
        ovi.fps_num         = 60000;
        ovi.fps_den         = 1000;
        ovi.graphics_module = DL_OPENGL;
        ovi.output_format   = VIDEO_FORMAT_I420;
        ovi.base_width      = 1920;
        ovi.base_height     = 1080;
        ovi.output_width    = 1920;
        ovi.output_height   = 1080;

        if(obs_reset_video(&ovi) != OBS_VIDEO_SUCCESS) {
            throw string("obs_reset_video failed");
        }

        oai.samples_per_sec  = 44100;
        oai.speakers         = SPEAKERS_STEREO;

        if (obs_reset_audio(&oai) != true) {
            throw string("obs_reset_audio failed");
        }

        obs_load_all_modules();


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
        fade_transition_A = obs_source_create("fade_transition", "fade transition", NULL, nullptr);
        if (!fade_transition_A) {
            throw string("Couldn't create fade_transition_A");
        }

        fade_transition_B = obs_source_create("fade_transition", "fade transition", NULL, nullptr);
        if (!fade_transition_B) {
            throw string("Couldn't create fade_transition_B");
        }


        obs_transition_set(fade_transition_A, rtmp_source_A);
        obs_transition_set(fade_transition_B, rtmp_source_B);


        // scenes (contain transitions)
        scene_A = obs_scene_create("test scene");
        if (!scene_A) {
            throw string("Couldn't create scene_A");
        }
        scene_item_A = obs_scene_add(scene_A, fade_transition_A);




        scene_B = obs_scene_create("test scene2");
        if (!scene_B) {
            throw string("Couldn't create scene_B");
        }
        scene_item_B = obs_scene_add(scene_B, fade_transition_B);



        // output
        output = obs_output_create("rtmp_output", "rtmp output", NULL, nullptr);
        if (!output) {
            throw string("Couldn't create output");
        }

        // encoders
        enc_a = obs_audio_encoder_create("libfdk_aac", "aac enc", NULL, 0, nullptr);
        if (!enc_a) {
            throw string("Couldn't create enc_a");
        }

        enc_v = obs_video_encoder_create("obs_x264", "h264 enc", NULL, nullptr);
        if (!enc_v) {
            throw string("Couldn't create enc_v");
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

                if(obs_transition_start(fade_transition_A, OBS_TRANSITION_MODE_AUTO, settings.transition_duration_ms, fade_transition_B) != true) {
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
        obs_source_release(fade_transition_A);
        obs_source_release(fade_transition_B);
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
