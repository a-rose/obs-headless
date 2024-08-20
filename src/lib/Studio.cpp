#include "Studio.hpp"
#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <obs-nix-platform.h>

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

Status Studio::StudioGet(ServerContext* ctx, const Empty* req, proto::StudioGetResponse* rep) {
	Status s = Status::OK;

	trace("Studio (get)");
	mtx.lock();
	try {
		proto::StudioState* proto_studio = rep->mutable_studio();

		if(active_show) {
			proto_studio->set_active_show_id(active_show->Id());
		} else {
			proto_studio->set_active_show_id("");
		}

		ShowMap::iterator it;
		for (it = shows.begin(); it != shows.end(); it++) {
			Show* show = it->second;
			proto::Show* proto_show = proto_studio->add_shows();
			if(show) {
				s = show->UpdateProto(proto_show);
				if(!s.ok()) {
					trace_error("Failed to update show proto", field_ns("id", show->Id()), field_ns("name", show->Name()));
					break;
				}
			} else {
				trace_error("NULL show", field_ns("id", it->first));
				s = Status(grpc::INTERNAL, "NULL show with id="+ it->first);
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::StudioStart(ServerContext* ctx, const Empty* req, Empty* rep) {
	Status s = Status::OK;

	trace("StudioStart");
	mtx.lock();
	try {
		if(!active_show) {
			s = Status(grpc::FAILED_PRECONDITION, "No active show");
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
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::StudioStop(ServerContext* ctx, const Empty* req, Empty* rep) {
	Status s = Status::OK;

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
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

///////////////////////////////////////
// SHOW                              //
///////////////////////////////////////

Status Studio::ShowGet(ServerContext* ctx, const proto::ShowGetRequest* req, proto::ShowGetResponse* rep) {
	Status s = Status::OK;

	trace("Show (get)");
	mtx.lock();
	try {
		string show_id = req->show_id();

		Show* show = getShow(show_id);
		if(show) {
			proto::Show* proto_show = rep->mutable_show();
			s = show->UpdateProto(proto_show);
		} else {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND, "Show not found: id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::ShowCreate(ServerContext* ctx, const proto::ShowCreateRequest* req, proto::ShowCreateResponse* rep) {
	Status s = Status::OK;

	trace("ShowCreate");
	mtx.lock();
	try {
		string show_name = req->show_name();
		Show* show = addShow(show_name);
		if(!show) {
			trace_error("Failed to create show");
			s = Status(grpc::INTERNAL, "Failed to create show");
		} else {
			proto::Show* proto_show = rep->mutable_show();
			s = show->UpdateProto(proto_show);
			trace_info("Created show", field_s(show_name));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::ShowDuplicate(ServerContext* ctx, const proto::ShowDuplicateRequest* req, proto::ShowDuplicateResponse* rep) {
	Status s = Status::OK;

	trace("ShowDuplicate");
	mtx.lock();
	try {
		string show_id = req->show_id();
		Show* show = duplicateShow(show_id);
		if(!show) {
			trace_error("Failed to duplicate show");
			s = Status(grpc::INTERNAL, "Failed to duplicate show");
		} else {
			proto::Show* proto_show = rep->mutable_show();
			s = show->UpdateProto(proto_show);
			trace_info("Duplicated show", field_s(show_id));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::ShowRemove(ServerContext* ctx, const proto::ShowRemoveRequest* req, Empty* rep) {
	Status s = Status::OK;

	trace("ShowRemove");
	mtx.lock();
	try {
		string show_id = req->show_id();
		s = removeShow(show_id);
		if(!s.ok()) {
			trace_error("Error during removeShow", error(s.error_message()));
		} else {
			trace_info("Removed show", field_s(show_id));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::ShowLoad(ServerContext* ctx, const proto::ShowLoadRequest* req, proto::ShowLoadResponse* rep) {
	Status s = Status::OK;

	trace("ShowLoad");
	mtx.lock();
	try {
		string show_path = req->show_path();
		Show* show = loadShow(show_path);

		if(!show) {
			trace_error("Failed to load show", field_s(show_path));
			s = Status(grpc::INTERNAL, "Failed to load show");
		} else {
			proto::Show* proto_show = rep->mutable_show();
			s = show->UpdateProto(proto_show);
			trace_info("Loaded show", field_s(show_path));
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

///////////////////////////////////////
// SCENE                             //
///////////////////////////////////////

Status Studio::SceneGet(ServerContext* ctx, const proto::SceneGetRequest* req, proto::SceneGetResponse* rep) {
	Status s = Status::OK;

	trace("Scene (get)");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		Show* show = getShow(show_id);

		if(show) {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = Status(grpc::NOT_FOUND, "Scene not found: id="+ scene_id);
			} else {
				proto::Scene* proto_scene = rep->mutable_scene();
				s = scene->UpdateProto(proto_scene);
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND,"Show not found: id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SceneAdd(ServerContext* ctx, const proto::SceneAddRequest* req, proto::SceneAddResponse* rep) {
	Status s = Status::OK;

	trace("SceneAdd");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_name = req->scene_name();
		Show* show = getShow(show_id);

		if(show) {
			Scene* scene = show->AddScene(scene_name);
			if(!scene) {
				trace_error("Failed to add scene", field_s(scene_name));
				s = Status(grpc::INTERNAL, "Failed to add scene");
			} else {
				proto::Scene* proto_scene = rep->mutable_scene();
				s = scene->UpdateProto(proto_scene);
				trace_info("Added scene", field_s(show_id), field_s(scene_name));
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND,"Show not found: id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SceneDuplicate(ServerContext* ctx, const proto::SceneDuplicateRequest* req, proto::SceneDuplicateResponse* rep) {
	Status s = Status::OK;

	trace("SceneDuplicate");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		Show* show = getShow(show_id);

		if(show) {
			Scene* new_scene = show->DuplicateScene(scene_id);
			if(!new_scene) {
				trace_error("Failed to duplicate scene", field_s(scene_id));
				s = Status(grpc::INTERNAL, "Failed to duplicate scene");
			} else {
				proto::Scene* proto_scene = rep->mutable_scene();
				s = new_scene->UpdateProto(proto_scene);
				trace_info("Duplicated scene", field_s(show_id), field_s(scene_id));
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND,"Show not found: id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SceneRemove(ServerContext* ctx, const proto::SceneRemoveRequest* req, Empty* rep) {
	Status s = Status::OK;

	trace("SceneRemove");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND, "Show not found id="+ show_id);
		} else {
			s = show->RemoveScene(scene_id);
			if(!s.ok()) {
				trace_error("Error in RemoveScene", field_s(show_id), field_s(scene_id));
			} else {
				trace_info("Removed scene", field_s(show_id), field_s(scene_id));
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SceneSetAsCurrent(ServerContext* ctx, const proto::SceneSetAsCurrentRequest* req, proto::SceneSetAsCurrentResponse* rep) {
	Status s = Status::OK;

	trace("SceneSetAsCurrent");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		Show* show = getShow(show_id);

		if(show) {
			s = show->SwitchScene(scene_id);
			if(s.ok()) {
				proto::Show* proto_show = rep->mutable_show();
				s = show->UpdateProto(proto_show);
				trace_info("Scene set as current", field_s(show_id), field_s(scene_id));
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND,"Show not found: id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SceneGetCurrent(ServerContext* ctx, const proto::SceneGetCurrentRequest* req, proto::SceneGetCurrentResponse* rep) {
	Status s = Status::OK;

	trace("SceneGetCurrent");
	mtx.lock();
	try {
		string show_id = req->show_id();
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND, "Show not found id="+ show_id);
		} else {
			Scene* active_scene = show->ActiveScene();
			if(!active_scene) {
				trace_error("null active scene in show", field_s(show_id));
				s = Status(grpc::INTERNAL, "null active scene in show id=", show_id);
			} else {
				rep->set_scene_id(active_scene->Id());
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

///////////////////////////////////////
// SOURCE                            //
///////////////////////////////////////

Status Studio::SourceGet(ServerContext* ctx, const proto::SourceGetRequest* req, proto::SourceGetResponse* rep) {
	Status s = Status::OK;

	trace("Source (get)");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		string source_id = req->source_id();
		Show* show = getShow(show_id);

		if(show) {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = Status(grpc::NOT_FOUND, "Scene not found: id="+ scene_id);
			} else {
				Source* source = scene->GetSource(source_id);
				if(!source) {
					trace_error("Source not found", field_s(source_id));
					s = Status(grpc::NOT_FOUND, "Source not found: id="+ source_id);
				} else {
					proto::Source* proto_source = rep->mutable_source();
					s = source->UpdateProto(proto_source);
				}
			}
		} else {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND, "Show not found: id="+ show_id);
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SourceAdd(ServerContext* ctx, const proto::SourceAddRequest* req, proto::SourceAddResponse* rep) {
	Status s = Status::OK;

	trace("SourceAdd");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		string source_name = req->source_name();
		string source_type = req->source_type();
		string source_url = req->source_url();
		Show* show = getShow(show_id);

		SourceType type = StringToSourceType(source_type);

		if(type == InvalidType) {
			trace_error("Unsupported type", field_s(source_type));
			s = grpc::Status(grpc::INVALID_ARGUMENT, "Unsupported type="+ type);
		} else {
			if(!show) {
				trace_error("Show not found", field_s(show_id));
				s = Status(grpc::NOT_FOUND, "Show not found id="+ show_id);
			} else {
				Scene* scene = show->GetScene(scene_id);

				if(!scene) {
					trace_error("Scene not found", field_s(scene_id));
					s = Status(grpc::NOT_FOUND, "Scene not found id="+ scene_id);
				} else {
					// TODO width and height
					Source* source = scene->AddSource(source_name, type, source_url, -1, -1);
					if(!source) {
						trace_error("Failed to add source", field_s(source_name));
						s = Status(grpc::INTERNAL, "Failed to add source");
					} else {
						proto::Source* proto_source = rep->mutable_source();
						s = source->UpdateProto(proto_source);
						trace_info("Added source", field_s(show_id), field_s(scene_id), field_s(source_name), field_s(source_url));
					}
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SourceDuplicate(ServerContext* ctx, const proto::SourceDuplicateRequest* req, proto::SourceDuplicateResponse* rep) {
	Status s = Status::OK;

	trace("SourceDuplicate");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		string source_id = req->source_id();
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found id", field_s(show_id));
			s = Status(grpc::NOT_FOUND, "Show not found id="+ show_id);
		} else {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found id", field_s(scene_id));
				s = Status(grpc::NOT_FOUND, "Scene not found id="+ scene_id);
			} else {
				Source* source = scene->DuplicateSource(source_id);
				if(!source) {
					trace_error("Failed to duplicate source", field_s(source_id));
					s = Status(grpc::INTERNAL, "Failed to duplicate source id="+ source_id);
				} else {
					proto::Source* proto_source = rep->mutable_source();
					s = source->UpdateProto(proto_source);
					trace_info("Duplicated source", field_s(show_id), field_s(scene_id), field_s(source_id));
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SourceRemove(ServerContext* ctx, const proto::SourceRemoveRequest* req, Empty* rep) {
	Status s = Status::OK;

	trace("SourceRemove");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		string source_id = req->source_id();
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND, "Show not found id="+ show_id);
		} else {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = Status(grpc::NOT_FOUND, "Scene not found id="+ scene_id);
			} else {
				s = scene->RemoveSource(source_id);
				if(!s.ok()) {
					trace_error("Error in RemoveSource", field_s(show_id), field_s(scene_id), field_s(source_id))
				} else {
					trace_info("Removed source", field_s(show_id), field_s(scene_id), field_s(source_id));
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::SourceSetProperties(ServerContext* ctx, const proto::SourceSetPropertiesRequest* req, proto::SourceSetPropertiesResponse* rep) {
	Status s = Status::OK;

	trace("SourceSetProperties");
	mtx.lock();
	try {
		string show_id = req->show_id();
		string scene_id = req->scene_id();
		string source_id = req->source_id();
		string source_type = req->source_type();
		string source_url = req->source_url();
		Show* show = getShow(show_id);

		if(!show) {
			trace_error("Show not found", field_s(show_id));
			s = Status(grpc::NOT_FOUND, "Show not found id=", show_id);
		} else {
			Scene* scene = show->GetScene(scene_id);

			if(!scene) {
				trace_error("Scene not found", field_s(scene_id));
				s = Status(grpc::NOT_FOUND, "Scene not found id="+ scene_id);
			} else if(scene == show->ActiveScene()) {
				trace_error("Scene is currently active", field_s(scene_id));
				s = Status(grpc::INVALID_ARGUMENT, "Invalid scene id");
			} else {
				Source* source = scene->GetSource(source_id);

				if(!source) {
					trace_error("Source not found", field_s(source_id));
					s = Status(grpc::NOT_FOUND, "Source not found id="+ source_id);
				} else {
					s = source->SetUrl(source_url);
					if(s.ok()) {
						s = source->SetType(source_type);
						if(s.ok()) {
							proto::Source* proto_source = rep->mutable_source();
							s = source->UpdateProto(proto_source);
							trace_info("Set properties for source", field_s(show_id), field_s(scene_id), field_s(source_id), field_s(source_type), field_s(source_url));
						} else {
							trace_error("Source SetType failed", field_s(source_id), field_s(source_type), error(s.error_message()));
						}
					} else {
						trace_error("Source SetUrl failed", field_s(source_id), field_s(source_url), error(s.error_message()));
					}
				}
			}
		}
	}
	catch(string e) {
		trace_error("An exception occured", error(e));
		s = Status(grpc::INTERNAL, e.c_str());
	}
	catch(...) {
		trace_error("An uncaught exception occured !");
		s = Status(grpc::INTERNAL, "An uncaught exception occured !");
	}
	mtx.unlock();

	return s;
}

Status Studio::Health(ServerContext* ctx, const Empty* req, proto::HealthResponse* rep) {
	trace("Health");
	rep->set_timestamp(std::time(nullptr));
	return Status::OK;
}

//////////////////////
// Private          //
//////////////////////

Status Studio::studioInit() {

	if(init) {
		return Status(grpc::FAILED_PRECONDITION, "Studio already initialized");
	}

	// OBS 27+: we need to set the display manually. The OBS app does it using
	// QT, so we do the same here.
	QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
	obs_set_nix_platform_display(native->nativeResourceForIntegration("display"));

	///////////////
	// OBS init  //
	///////////////
	if(!obs_startup("en-US", nullptr, nullptr) || !obs_initialized()) {
		return Status(grpc::INTERNAL, "obs_startup failed");
	}

	memset(&ovi, 0, sizeof(ovi));
	memset(&oai, 0, sizeof(oai));

	ovi.adapter         = 0;
	ovi.graphics_module = LIBOBS_PATH"libobs-opengl.so";
	ovi.output_format   = VIDEO_FORMAT_NV12; // TODO to settings with VIDEO_FORMAT_I420
	ovi.fps_num         = settings->video_fps_num;
	ovi.fps_den         = settings->video_fps_den;
	ovi.base_width      = settings->video_width;
	ovi.base_height     = settings->video_height;
	ovi.output_width    = settings->video_width;
	ovi.output_height   = settings->video_height;
	ovi.gpu_conversion  = settings->video_gpu_conversion;

	trace_debug("", field_s(ovi.graphics_module));

	if(obs_reset_video(&ovi) != OBS_VIDEO_SUCCESS) {
		return Status(grpc::INTERNAL, "obs_reset_video failed");
	}

	oai.samples_per_sec  = settings->audio_sample_rate;
	oai.speakers         = SPEAKERS_STEREO; // TODO to settings

	if (obs_reset_audio(&oai) != true) {
		return Status(grpc::INTERNAL, "obs_reset_audio failed");
	}

	// Load modules
	// For color_source
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "image-source.so", LIBOBS_PLUGINS_DATA_PATH "image-source")) {
		return Status(grpc::INTERNAL, "failed to load lib image-source.so");
	}
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-ffmpeg.so", LIBOBS_PLUGINS_DATA_PATH "obs-ffmpeg")) {
		return Status(grpc::INTERNAL, "failed to load lib obs-ffmpeg.so");
	}
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-transitions.so", LIBOBS_PLUGINS_DATA_PATH "obs-transitions")) {
		return Status(grpc::INTERNAL, "failed to load lib obs-transitions.so");
	}
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "rtmp-services.so", LIBOBS_PLUGINS_DATA_PATH "rtmp-services")) {
		return Status(grpc::INTERNAL, "failed to load lib rtmp-services.so");
	}
	if(!settings->video_hw_encode) {
		if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-x264.so", LIBOBS_PLUGINS_DATA_PATH "obs-x264")) {
			return Status(grpc::INTERNAL, "failed to load lib obs-x264.so");
		}
	}
	// For fdk-aac
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-libfdk.so", LIBOBS_PLUGINS_DATA_PATH "obs-libfdk")) {
		return Status(grpc::INTERNAL, "failed to load lib obs-libfdk.so");
	}
	// For rtmp-output
	if(0 != loadModule(LIBOBS_PLUGINS_PATH "obs-outputs.so", LIBOBS_PLUGINS_DATA_PATH "obs-outputs")) {
		return Status(grpc::INTERNAL, "failed to load lib obs-outputs.so");
	}

	obs_post_load_modules();

	// output and service	
	service = obs_service_create("rtmp_common", "rtmp service", nullptr, nullptr);
	if (!service) {
		return Status(grpc::INTERNAL, "Couldn't create service");
	}

	rtmp_settings = obs_data_create();
	if (!rtmp_settings) {
		return Status(grpc::INTERNAL, "Couldn't create rtmp settings");
	}

	obs_data_set_string(rtmp_settings, "server", settings->server.c_str());
	obs_data_set_string(rtmp_settings, "key", settings->key.c_str());
	obs_service_update(service, rtmp_settings);
	obs_data_release(rtmp_settings);

	output = obs_output_create("rtmp_output", "RTMP output", NULL, nullptr);
	if (!output) {
		return Status(grpc::INTERNAL, "Couldn't create output");
	}


	// Audio encoder
	enc_a = obs_audio_encoder_create("libfdk_aac", "aac enc", NULL, 0, nullptr);
	if (!enc_a) {
		return Status(grpc::INTERNAL, "Couldn't create enc_a");
	}

	enc_a_settings = obs_encoder_get_settings(enc_a);
	if (!enc_a_settings) {
		return Status(grpc::INTERNAL, "Failed to create enc_a_settings");
	}

	obs_data_set_int(	enc_a_settings, "bitrate",		settings->audio_bitrate_kbps);
	obs_data_set_bool(	enc_a_settings, "afterburner",	true);
	obs_encoder_update(enc_a, enc_a_settings);
	obs_data_release(enc_a_settings);

	// Video encoder
	string encoder = settings->video_hw_encode ? "ffmpeg_nvenc" : "obs_x264";
	enc_v = obs_video_encoder_create(encoder.c_str(), "h264 enc", NULL, nullptr);
	if (!enc_v) {
		return Status(grpc::INTERNAL, "Couldn't create enc_v");
	}

	enc_v_settings = obs_encoder_get_settings(enc_v);
	if (!enc_v_settings) {
		return Status(grpc::INTERNAL, "Failed to create enc_v_settings");
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
	grpc::Status s = active_show->Start();
	if(!s.ok()) {
		return s;
	}

	obs_set_output_source(0, active_show->Transition());
	obs_encoder_set_video(enc_v, obs_get_video());
	obs_encoder_set_audio(enc_a, obs_get_audio());
	obs_output_set_video_encoder(output, enc_v);
	obs_output_set_audio_encoder(output, enc_a, 0);
	obs_output_set_service(output, service);

	if(obs_output_start(output) != true) {
		s = Status(grpc::INTERNAL, "obs_output_start failed: "+ std::string(obs_output_get_last_error(output)));
	}

	init = true;
	return Status::OK;
}

Status Studio::studioRelease() {
	if(!init) {
		return Status(grpc::FAILED_PRECONDITION, "Studio not started");
	}

	obs_encoder_release(enc_v);
	obs_encoder_release(enc_a);
	obs_service_release(service);
	obs_output_release(output);

	Status s = active_show->Stop();
	if(!s.ok()) {
		return s;
	}

	obs_shutdown();
	init = false;
	trace("StudioStop Ok !");
	return Status::OK;
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
	Status s;
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

	s = show->Load(json_show);
	json_decref(json_show);

	if(!s.ok()) {
		trace_error("Error during show Load", error(s.error_message()));
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

Status Studio::removeShow(string show_id) {
	ShowMap::iterator it = shows.find(show_id);
	if(it == shows.end()) {
		return Status(grpc::NOT_FOUND, "Show not found id="+ show_id);
	}
	if(it->second == active_show) {
		return Status(grpc::FAILED_PRECONDITION, "Show is active id="+ show_id);
	}

	trace_debug("Remove show", field_s(show_id));
	// No need to do show->Stop(); because it is not actve
	delete it->second;
	shows.erase(it);

	return Status::OK;
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
