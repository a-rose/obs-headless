#include "Studio.hpp"

Studio::Studio(Settings* settings_in)
	: settings(settings_in)
	, active_show(nullptr)
	, init(false)
	, show_id_counter(0) {
}

Studio::~Studio() {
	trace("Studio destructor");
	ShowMap::iterator it;
	for (it = shows.begin(); it != shows.end(); it++) {
		Show* show = it->second;
		if(!show) {
			trace_debug("NULL show", field_ns("id", it->first));
			continue;
		}
		trace_debug("delete show", field_ns("id", show->Id()));
		delete show;
	}
}

///////////////////////////////////////
// STUDIO                            //
///////////////////////////////////////

// TODO reimplement properly
StudioStatus Studio::GetStudio() {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("Studio (get)");
	mtx.lock();
	try {
		ShowMap::iterator it;
		for (it = shows.begin(); it != shows.end(); it++) {
			Show* show = it->second;
			if(!show) {
				trace_error("NULL show", field_ns("id", it->first));
				s = StudioStatus(STUDIO_SHOW_NOT_FOUND, std::string("NULL show with id="+ it->first));
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::StudioStart() {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("StudioStart");
	mtx.lock();
	try {
		if(!active_show) {
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "No active show");
				trace_error("No active show");
		} else {
			s = studioInit();
			if(!s.ok()) {
				trace_error("Error during studioInit", error(s.error_message()));
			} else {
				trace_info("Started studio");
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::StudioStop() {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("StudioStop");
	mtx.lock();
	try {
		s = studioRelease();
		if(!s.ok()) {
			trace_error("Error during studioRelease", error(s.error_message()));
		} else {
			trace_info("Stopped studio");
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

///////////////////////////////////////
// SHOW                              //
///////////////////////////////////////

StudioStatus Studio::GetShow(string show_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("Show (get)");
	mtx.lock();
	try {
		Show* show = getShow(show_id);
		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "Show not found: id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::ShowCreate(string show_name) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("ShowCreate");
	mtx.lock();
	try {
		Show* show = addShow(show_name);
		if(!show) {
			trace_error("Failed to create show");
			s = StudioStatus(STUDIO_ERROR, "Failed to create show");
		} else {
			trace_info("Created show", field_s(show_name));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::ShowDuplicate(string show_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("ShowDuplicate");
	mtx.lock();
	try {
		Show* show = duplicateShow(show_id);
		if(!show) {
			trace_error("Failed to duplicate show");
			s = StudioStatus(STUDIO_ERROR, "Failed to duplicate show");
		} else {
			trace_info("Duplicated show", field_s(show_id));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::ShowRemove(string show_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("ShowRemove");
	mtx.lock();
	try {
		s = removeShow(show_id);
		if(!s.ok()) {
			trace_error("Error during removeShow", error(s.error_message()));
		} else {
			trace_info("Removed show", field_s(show_id));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}
StudioStatus Studio::ShowLoad(string show_path) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("ShowLoad");
	mtx.lock();
	try {
		Show* show = loadShow(show_path);

		if(!show) {
			s = StudioStatus(STUDIO_ERROR, "Failed to load show");
		} else {
			trace_info("Loaded show", field_s(show_path));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

///////////////////////////////////////
// SCENE                             //
///////////////////////////////////////

StudioStatus Studio::GetScene(string show_id, string scene_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("Scene (get)");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(show) {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = StudioStatus(STUDIO_SCENE_NOT_FOUND, "id="+ scene_id);
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND,"id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SceneAdd(string show_id, string scene_name) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SceneAdd");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(show) {
			Scene* scene = show->AddScene(scene_name);
			if(!scene) {
				trace_error("Failed to add scene", field_s(scene_name));
				s = StudioStatus(STUDIO_ERROR, "Failed to add scene");
			} else {
				trace_info("Added scene", field_s(show_id), field_s(scene_name));
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SceneDuplicate(string show_id, string scene_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SceneDuplicate");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(show) {
			Scene* new_scene = show->DuplicateScene(scene_id);
			if(!new_scene) {
				trace_error("Failed to duplicate scene", field_s(scene_id));
				s = StudioStatus(STUDIO_ERROR, "Failed to duplicate scene");
			} else {
				trace_info("Duplicated scene", field_s(show_id), field_s(scene_id));
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SceneRemove(string show_id, string scene_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SceneRemove");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
		} else {
			ShowStatus sh = show->RemoveScene(scene_id);
			if(!sh.ok()) {
				trace_error("Error in RemoveScene", field_s(show_id), field_s(scene_id));
				s = StudioStatus(STUDIO_ERROR, s.error_message());
			} else {
				trace_info("Removed scene", field_s(show_id), field_s(scene_id));
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SceneSetAsCurrent(string show_id, string scene_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SceneSetAsCurrent");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(show) {
			ShowStatus sh = show->SwitchScene(scene_id);
			if(sh.ok()) {
				trace_info("Scene set as current", field_s(show_id), field_s(scene_id));
			} else {
				s = StudioStatus(STUDIO_ERROR, sh.error_message());
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SceneGetCurrent(string show_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SceneGetCurrent");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, std::string("id="+ show_id));
		} else {
			Scene* active_scene = show->ActiveScene();
			if(!active_scene) {
				trace_error("null active scene in show", field_s(show_id));
				s = StudioStatus(STUDIO_ERROR, std::string("null active scene in show id="+ show_id));
			} else {
				// TODO trace
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

///////////////////////////////////////
// SOURCE                            //
///////////////////////////////////////

StudioStatus Studio::GetSource(string show_id, string scene_id, string source_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("Source (get)");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(show) {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = StudioStatus(STUDIO_SCENE_NOT_FOUND, "id="+ scene_id);
			} else {
				Source* source = scene->GetSource(source_id);
				if(!source) {
					trace_error("Source not found", field_s(source_id));
					s = StudioStatus(STUDIO_SOURCE_NOT_FOUND, "id="+ source_id);
				} else {
					// TODO trace
				}
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, " id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SourceAdd(string show_id, string scene_id, string source_name, string source_type, string source_url) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SourceAdd");
	mtx.lock();
	try {
		Show* show = getShow(show_id);
		SourceType type = StringToSourceType(source_type);

		if(type == InvalidType) {
			trace_error("Unsupported type", field_s(source_type));
			s = StudioStatus(STUDIO_ERROR, "Unsupported type="+ std::to_string(type));
		} else {
			if(!show) {
				trace_error("Show not found", field_s(show_id));
				s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
			} else {
				Scene* scene = show->GetScene(scene_id);

				if(!scene) {
					trace_error("Scene not found", field_s(scene_id));
					s = StudioStatus(STUDIO_SCENE_NOT_FOUND, "d="+ scene_id);
				} else {
					Source* source = scene->AddSource(source_name, type, source_url);
					if(!source) {
						trace_error("Failed to add source", field_s(source_name));
						s = StudioStatus(STUDIO_ERROR, "Failed to add source");
					} else {
						trace_info("Added source", field_s(show_id), field_s(scene_id), field_s(source_name), field_s(source_url));
					}
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SourceDuplicate(string show_id, string scene_id, string source_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SourceDuplicate");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found id", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
		} else {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found id", field_s(scene_id));
				s = StudioStatus(STUDIO_SCENE_NOT_FOUND, "d="+ scene_id);
			} else {
				Source* source = scene->DuplicateSource(source_id);
				if(!source) {
					trace_error("Failed to duplicate source", field_s(source_id));
					s = StudioStatus(STUDIO_ERROR, "Failed to duplicate source id="+ source_id);
				} else {
					trace_info("Duplicated source", field_s(show_id), field_s(scene_id), field_s(source_id));
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SourceRemove(string show_id, string scene_id, string source_id) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SourceRemove");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
		} else {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = StudioStatus(STUDIO_SCENE_NOT_FOUND, std::string("d="+ scene_id));
			} else {
				SceneStatus sc = scene->RemoveSource(source_id);
				if(!sc.ok()) {
					trace_error("Error in RemoveSource", field_s(show_id), field_s(scene_id), field_s(source_id))
					s = StudioStatus(STUDIO_ERROR, sc.error_message());
				} else {
					trace_info("Removed source", field_s(show_id), field_s(scene_id), field_s(source_id));
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

StudioStatus Studio::SourceSetProperties(string show_id, string scene_id, string source_id, string source_type, string source_url) {
	StudioStatus s = StudioStatus(STUDIO_OK);

	trace("SourceSetProperties");
	mtx.lock();
	try {
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = StudioStatus(STUDIO_SHOW_NOT_FOUND, std::string("id="+ show_id));
		} else {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = StudioStatus(STUDIO_SCENE_NOT_FOUND, "d="+ scene_id);
			} else if(scene == show->ActiveScene()) {
				trace_error("Scene is currently active", field_s(scene_id));
				s = StudioStatus(STUDIO_SCENE_NOT_FOUND, "Scene is currently active");
			} else {
				Source* source = scene->GetSource(source_id);

				if(!source) {
					trace_error("Source not found", field_s(source_id));
					s = StudioStatus(STUDIO_SOURCE_NOT_FOUND, "id="+ source_id);
				} else {
					SourceStatus so = source->SetUrl(source_url);
					if(so.ok()) {
						so = source->SetType(source_type);
						if(so.ok()) {
							trace_info("Set properties for source", field_s(show_id), field_s(scene_id), field_s(source_id), field_s(source_type), field_s(source_url));
						} else {
							trace_error("Source SetType failed", field_s(source_id), field_s(source_type), error(so.error_message()));
							s = StudioStatus(STUDIO_ERROR, so.error_message());
						}
					} else {
						trace_error("Source SetUrl failed", field_s(source_id), field_s(source_url), error(s.error_message()));
						s = StudioStatus(STUDIO_ERROR, so.error_message());
					}
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = StudioStatus(STUDIO_ERROR, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = StudioStatus(STUDIO_ERROR, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}


//////////////////////
// Private          //
//////////////////////

StudioStatus Studio::studioInit() {

	if(init) {
		return StudioStatus(STUDIO_ALREADY_STARTED);
	}

	///////////////
	// OBS init  //
	///////////////
	if(!obs_startup("en-US", nullptr, nullptr) || !obs_initialized()) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "obs_startup failed");
	}

	memset(&ovi, 0, sizeof(ovi));
	memset(&oai, 0, sizeof(oai));

	ovi.adapter         = 0;
	ovi.graphics_module = LIBOBS_PATH"libobs-opengl.so.0.0";
	ovi.output_format   = VIDEO_FORMAT_NV12; // TODO to settings with VIDEO_FORMAT_I420
	ovi.fps_num         = settings->video_fps_num;
	ovi.fps_den         = settings->video_fps_den;
	ovi.base_width      = settings->video_width;
	ovi.base_height     = settings->video_height;
	ovi.output_width    = settings->video_width;
	ovi.output_height   = settings->video_height;
	ovi.gpu_conversion  = settings->video_gpu_conversion;

	if(obs_reset_video(&ovi) != OBS_VIDEO_SUCCESS) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "obs_reset_video failed");
	}

	oai.samples_per_sec  = settings->audio_sample_rate;
	oai.speakers         = SPEAKERS_STEREO; // TODO to settings

	if (obs_reset_audio(&oai) != true) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "obs_reset_audio failed");
	}

	// Load modules
	// For color_source
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "image-source.so", LIBOBS_PLUGINS_DATA_PATH "image-source")) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "failed to load lib image-source.so");
	}
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-ffmpeg.so", LIBOBS_PLUGINS_DATA_PATH "obs-ffmpeg")) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "failed to load lib obs-ffmpeg.so");
	}
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-transitions.so", LIBOBS_PLUGINS_DATA_PATH "obs-transitions")) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "failed to load lib obs-transitions.so");
	}
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "rtmp-services.so", LIBOBS_PLUGINS_DATA_PATH "rtmp-services")) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "failed to load lib rtmp-services.so");
	}
	if(!settings->video_hw_encode) {
		if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-x264.so", LIBOBS_PLUGINS_DATA_PATH "obs-x264")) {
			return StudioStatus(STUDIO_LIBOBS_ERROR, "failed to load lib obs-x264.so");
		}
	}
	// For fdk-aac
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-libfdk.so", LIBOBS_PLUGINS_DATA_PATH "obs-libfdk")) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "failed to load lib obs-libfdk.so");
	}
	// For rtmp-output
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-outputs.so", LIBOBS_PLUGINS_DATA_PATH "obs-outputs")) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "failed to load lib obs-outputs.so");
	}

	obs_post_load_modules();

	// output and service	
	service = obs_service_create("rtmp_common", "rtmp service", rtmp_settings, nullptr);
	if (!service) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "Couldn't create service");
	}

	rtmp_settings = obs_service_get_settings(service);
	if (!rtmp_settings) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "Couldn't create rtmp settings");
	}

	obs_data_set_string(rtmp_settings, "service", settings->server.c_str());
	obs_data_set_string(rtmp_settings, "key", settings->key.c_str());
	obs_service_update(service, rtmp_settings); // TODO test it works fine

	output = obs_output_create("rtmp_output", "RTMP output", NULL, nullptr);
	if (!output) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "Couldn't create output");
	}
	obs_data_release(rtmp_settings);


	// Audio encoder
	enc_a = obs_audio_encoder_create("libfdk_aac", "aac enc", NULL, 0, nullptr);
	if (!enc_a) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "Couldn't create enc_a");
	}

	enc_a_settings = obs_encoder_get_settings(enc_a);
	if (!enc_a_settings) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "Failed to create enc_a_settings");
	}

	obs_data_set_int(	enc_a_settings, "bitrate",		settings->audio_bitrate_kbps);
	obs_data_set_bool(	enc_a_settings, "afterburner",	true);
	obs_encoder_update(enc_a, enc_a_settings);
	obs_data_release(enc_a_settings);

	// Video encoder
	string encoder = settings->video_hw_encode ? "ffmpeg_nvenc" : "obs_x264";
	enc_v = obs_video_encoder_create(encoder.c_str(), "h264 enc", NULL, nullptr);
	if (!enc_v) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "Couldn't create enc_v");
	}

	enc_v_settings = obs_encoder_get_settings(enc_v);
	if (!enc_v_settings) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "Failed to create enc_v_settings");
	}

	obs_data_set_int(	enc_v_settings, "bitrate",		settings->video_bitrate_kbps);
	obs_data_set_int(	enc_v_settings, "keyint_sec",	settings->video_keyint_sec);
	obs_data_set_string(enc_v_settings, "rate_control",	settings->video_rate_control.c_str());
	if(settings->video_hw_encode) {
		// TODO HW encoder settings
		// obs_data_set_int(	enc_v_settings, "max_bitrate",	settings->videoBitrateKbps);
		obs_data_set_string(enc_v_settings, "preset",		"default");
		obs_data_set_string(enc_v_settings, "profile",		"main");
		obs_data_set_int(	enc_v_settings, "bf",			2);
		obs_data_set_bool(	enc_v_settings, "psycho_aq",	false);
		obs_data_set_bool(	enc_v_settings, "lookahead",	false);
		// obs_data_set_int(	enc_v_settings, "cqp",			(1, 30, 1));
		// obs_data_set_int(	enc_v_settings, "gpu",			(0, 8, 1));
	} else {
		obs_data_set_int(	enc_v_settings, "width",		settings->video_width);
		obs_data_set_int(	enc_v_settings, "height",		settings->video_height);
		obs_data_set_int(	enc_v_settings, "fps_num",		settings->video_fps_num);
		obs_data_set_int(	enc_v_settings, "fps_den",		settings->video_fps_den);
		// TODO sw encoder settings
		// obs_data_set_int(enc_v_settings, "buffer_size",		settings->videoBitrateKbps);
		obs_data_set_string(enc_v_settings, "preset",		"ultrafast");
		obs_data_set_string(enc_v_settings, "profile",		"main");
		obs_data_set_string(enc_v_settings, "tune",			"zerolatency");
		obs_data_set_string(enc_v_settings, "x264opts",		"");
		// obs_data_set_bool(enc_v_settings, "use_bufsize",		false);
		// obs_data_set_int(enc_v_settings, "crf",		0);
		// #ifdef ENABLE_VFR
		// obs_data_set_bool(enc_v_settings, "vfr",		true);
		// #endif
	}
	obs_encoder_update(enc_v, enc_v_settings);
	obs_data_release(enc_v_settings);

	///////////////
	// Show init //
	///////////////
	ShowStatus sh = active_show->Start();
	if(!sh.ok()) {
		return StudioStatus(STUDIO_ERROR, sh.error_message());
	}

	obs_set_output_source(0, active_show->Transition());
	obs_encoder_set_video(enc_v, obs_get_video());
	obs_encoder_set_audio(enc_a, obs_get_audio());
	obs_output_set_video_encoder(output, enc_v);
	obs_output_set_audio_encoder(output, enc_a, 0);
	obs_output_set_service(output, service);

	if(obs_output_start(output) != true) {
		return StudioStatus(STUDIO_LIBOBS_ERROR, "obs_output_start failed: "+ std::string(obs_output_get_last_error(output)));
	}

	init = true;
	return StudioStatus(STUDIO_OK);
}

StudioStatus Studio::studioRelease() {
	if(!init) {
		return StudioStatus(STUDIO_ALREADY_STOPPED);
	}

	obs_encoder_release(enc_v);
	obs_encoder_release(enc_a);
	obs_service_release(service);
	obs_output_release(output);

	ShowStatus s = active_show->Stop();
	if(!s.ok()) {
		return StudioStatus(STUDIO_ERROR, s.error_message());
	}

	obs_shutdown();
	init = false;
	trace("StudioStop Ok !");
	return StudioStatus(STUDIO_OK);
}

Show* Studio::getShow(std::string show_id) {
	ShowMap::iterator it = shows.find(show_id);
	if (it == shows.end()) {
		return NULL;
	}
	return it->second;
}

Show* Studio::addShow(string show_name) {
	std::string show_id = "show_"+ std::to_string(show_id_counter);
	show_id_counter++;

	Show* show = new Show(show_id, show_name, settings);
	if(!show) {
		trace_error("Failed to create a show", field_s(show_id));
		return NULL;
	}

	trace_debug("Add show", field_s(show_id));
	shows[show_id] = show;
	// TODO need a setActive method
	if(!active_show) {
		active_show = show;
	}
	return show;
}

Show* Studio::loadShow(string show_id) {
	ShowStatus sh;
	Show* show;
	json_t* json_show;
	json_error_t error;

	json_show = json_load_file(show_id.c_str(), 0, &error);
	if(!json_show) {
		trace_error("Error while loading json config", field_s(show_id), field_nc("error", error.text));
		return NULL;
	}

	show = addShow(show_id);
	if(!show) {
		json_decref(json_show);
		trace_error("Error while creating show", field_s(show_id));
		return NULL;
	}

	sh = show->Load(json_show);
	json_decref(json_show);

	if(!sh.ok()) {
		trace_error("Error during show Load", error(sh.error_message()));
		delete show;
		return NULL;
	}

	return show;
}

Show* Studio::duplicateShow(string show_id) {
	Show* show = getShow(show_id);
	if(!show) {
		trace_error("Show not found", field_s(show_id));
		return NULL;
	}

	Show* new_show = addShow(show->Name());
	if(!new_show) {
		trace_error("Failed to duplicate show");
		return NULL;
	}

	SceneMap::iterator it;
	for (it = show->Scenes().begin(); it != show->Scenes().end(); it++) {
		Scene* scene = it->second;
		trace_debug("scene from original show", field_ns("id", scene->Id()), field_ns("name", scene->Name()));

		Scene* new_scene = new_show->DuplicateSceneFromShow(show, scene->Id());
		if(!new_scene) {
			trace_error("Failed to duplicate scene");
			break;
		}
	}

	return new_show;
}

StudioStatus Studio::removeShow(string show_id) {
	ShowMap::iterator it = shows.find(show_id);
	if(it == shows.end()) {
		return StudioStatus(STUDIO_SHOW_NOT_FOUND, "id="+ show_id);
	}
	if(it->second == active_show) {
		return StudioStatus(STUDIO_SHOW_ACTIVE, "id="+ show_id);
	}

	trace_debug("Remove show", field_s(show_id));
	// No need to do show->Stop(); because it is not actve
	delete it->second;
	shows.erase(it);

	return StudioStatus(STUDIO_OK);
}

int Studio::loadModule(const char* binPath, const char* dataPath) {
	obs_module_t *module;

	int code = obs_open_module(&module, binPath, dataPath);
	if (code != MODULE_SUCCESS) {
		trace_error("Failed to load module file",field_c(binPath), field(code));
		return -1;
	}

	if(obs_init_module(module) != true) {
		trace_error("obs_init_module failed");
		return -1;
	}

	return 0;
}
