#include "Source.hpp"

std::string SourceTypeToString(SourceType type) {
	switch(type) {
	case Image:
		return "Image";
	case RTMP:
		return "RTMP";
	}
	return "InvalidType";
}

SourceType StringToSourceType(std::string type) {
	if(type == "Image") {
		return Image;
	} else if(type == "RTMP") {
		return RTMP;
	}
	return InvalidType;
}

Source::Source(std::string id, std::string name, SourceType type, std::string url, int width, int height, Settings* settings)
	: id(id)
	, name(name)
	, type(type)
	, url(url)
	, width(width)
	, height(height)
	, started(false)
	, obs_source(nullptr)
	, obs_scene_ptr(nullptr)
	, settings(settings) {
	trace_debug("Create Source", field_s(id), field_s(name), field_ns("type", SourceTypeToString(type)), field_s(url));
}

Source::~Source() {
}

grpc::Status Source::SetType(std::string new_type) {
	if(started) {
		trace_error("Source already started", field_s(id));
		return grpc::Status(grpc::FAILED_PRECONDITION, "Source already started");
	}

	SourceType tempType = StringToSourceType(new_type);

	if(tempType == InvalidType) {
		trace_error("Unsupported type", field_s(new_type));
		return grpc::Status(grpc::INVALID_ARGUMENT, "Unsupported type="+ new_type);
	} else {
		type = tempType;
		trace_info("update source", field_s(id), field_s(name), field_ns("type", SourceTypeToString(type)));
	}

	return grpc::Status::OK;
}

grpc::Status Source::SetUrl(std::string new_url) {
	if(started) {
		trace_error("Source already started", field_s(id));
		return grpc::Status(grpc::FAILED_PRECONDITION, "Source already started");
	}
	url = new_url;
	trace_info("update source", field_s(id), field_s(name), field_s(url));
	return grpc::Status::OK;
}

grpc::Status Source::Start(obs_scene_t** obs_scene_in) {	
	grpc::Status s = grpc::Status::OK;
	obs_data_t* obs_data;
	obs_scene_ptr = obs_scene_in;

	if(started) {
		trace_error("Source already started", field_s(id));
		return grpc::Status(grpc::FAILED_PRECONDITION, "Source already started");
	}

	// Create the source
	obs_data = obs_data_create();
	if (!obs_data) {
		trace_error("Failed to create obs_data", field_s(id));
		return grpc::Status(grpc::INTERNAL, "Failed to create obs_data");
	}

	if(type == Image) {
		obs_data_set_string(obs_data, "file", url.c_str());
		obs_data_set_bool(obs_data, "unload", false);

		obs_source = obs_source_create("image_source", "obs_image_source", obs_data, nullptr);
	} else if(type == RTMP){
		trace_debug("create ffmpeg src", field_s(id), field_s(name), field_s(url));

		obs_data_set_string(obs_data, "input", url.c_str());
		obs_data_set_bool(obs_data, "is_local_file", false);
		obs_data_set_bool(obs_data, "looping", true);
		obs_data_set_bool(obs_data, "hw_decode", settings->video_hw_decode);

		std::string source_name = std::string("obs_src_ffmpeg_"+name);
		obs_source = obs_source_create("ffmpeg_source", source_name.c_str(), obs_data, nullptr);
	} else {
		trace_error("Unsupported source type", field(type));
	}

	obs_data_release(obs_data);

	if (!obs_source) {
		return grpc::Status(grpc::INTERNAL, "Failed to create obs_source");
	}

	// Add the source to the scene
	s = addSourceToScene(obs_source);
	if(!s.ok()) {
		return s;
	}

	// Register signals callbacks on the main source
	signal_handler_t *handler = obs_source_get_signal_handler(obs_source);
	signal_handler_connect(handler, "show", SourceShowCb, this);
	signal_handler_connect(handler, "hide", SourceHideCb, this);
	signal_handler_connect(handler, "activate", SourceActivateCb, this);
	signal_handler_connect(handler, "transition_start", SourceTransitionStartCb, this);
	signal_handler_connect(handler, "transition_video_stop", SourceTransitionVideoStopCb, this);
	signal_handler_connect(handler, "transition_stop", SourceTransitionStopCb, this);

	started = true;
	return grpc::Status::OK;
}

grpc::Status Source::Stop() {
	if(!started) {
		trace_error("Source already stopped", field_s(id));
		return grpc::Status(grpc::FAILED_PRECONDITION, "Source already stopped");
	}

	obs_source_release(obs_source);
	started = false;
	return grpc::Status::OK;
}

void SourceShowCb(void *my_data, calldata_t *cd) {
	Source* src				= (Source*) my_data;
	obs_source_t *obs_source	= (obs_source_t*) calldata_ptr(cd, "source");

	if(!src || !obs_source) {
		trace_error("sourcecb: source is null");
		return;
	}

	trace_debug("sourcecb: show", field_nc("id", obs_source_get_id(obs_source)), field_nc("name", obs_source_get_name(obs_source)));
}

void SourceHideCb(void *my_data, calldata_t *cd) {
	Source* src				= (Source*) my_data;
	obs_source_t *obs_source	= (obs_source_t*) calldata_ptr(cd, "source");

	if(!src || !obs_source) {
		trace_error("sourcecb: source is null");
		return;
	}

	trace_debug("sourcecb: hide", field_nc("id", obs_source_get_id(obs_source)), field_nc("name", obs_source_get_name(obs_source)));
}

void SourceActivateCb(void *my_data, calldata_t *cd) {
	Source* src				= (Source*) my_data;
	obs_source_t *obs_source	= (obs_source_t*) calldata_ptr(cd, "source");

	if(!src || !obs_source) {
		trace_error("sourcecb: source is null");
		return;
	}

	trace_debug("sourcecb: activate", field_nc("id", obs_source_get_id(obs_source)), field_nc("name", obs_source_get_name(obs_source)));
}

void SourceTransitionStartCb(void *my_data, calldata_t *cd) {
	Source* src				= (Source*) my_data;
	obs_source_t *obs_source	= (obs_source_t*) calldata_ptr(cd, "source");

	if(!src || !obs_source) {
		trace_error("sourcecb: source is null");
		return;
	}

	trace_debug("sourcecb: start transition source", field_nc("id", obs_source_get_id(obs_source)), field_nc("name", obs_source_get_name(obs_source)));
}

void SourceTransitionVideoStopCb(void *my_data, calldata_t *cd) {
	Source* src				= (Source*) my_data;
	obs_source_t *obs_source	= (obs_source_t*) calldata_ptr(cd, "source");

	if(!src || !obs_source) {
		trace_error("sourcecb: source is null");
		return;
	}

	trace_debug("sourcecb: stop video transition", field_nc("id", obs_source_get_id(obs_source)), field_nc("name", obs_source_get_name(obs_source)));
}

void SourceTransitionStopCb(void *my_data, calldata_t *cd) {
	Source* src				= (Source*) my_data;
	obs_source_t *obs_source	= (obs_source_t*) calldata_ptr(cd, "source");

	if(!src || !obs_source) {
		trace_error("sourcecb: source is null");
		return;
	}

	trace_debug("sourcecb: stop transition", field_nc("id", obs_source_get_id(obs_source)), field_nc("name", obs_source_get_name(obs_source)));
}

grpc::Status Source::UpdateProto(proto::Source* proto_source) {
	proto_source->Clear();
	proto_source->set_id(id);
	proto_source->set_name(name);
	proto_source->set_type(SourceTypeToString(type));
	proto_source->set_url(url);
	return grpc::Status::OK;
}

grpc::Status Source::addSourceToScene(obs_source_t* source) {
	obs_sceneitem_t* obs_scene_item = obs_scene_add(*obs_scene_ptr, source);
	if (!obs_scene_item) {
		trace_error("Error while adding scene item", field_s(id));
		return grpc::Status(grpc::INTERNAL, "Error while adding scene item");
	}

	// Scale source to output size by setting bounds
	struct vec2 bounds;
	if(width > 0 && height > 0) {
		bounds.x = width;
		bounds.y = height;
	} else {
		bounds.x = settings->video_width;
		bounds.y = settings->video_height;
	}
	uint32_t align = OBS_ALIGN_TOP + OBS_ALIGN_LEFT;
	obs_sceneitem_set_bounds_type(obs_scene_item, OBS_BOUNDS_SCALE_INNER);
	obs_sceneitem_set_bounds(obs_scene_item, &bounds);
	obs_sceneitem_set_bounds_alignment(obs_scene_item, align);

	return grpc::Status::OK;
}

grpc::Status Source::setSourceOrder(obs_source_t* source, enum obs_order_movement order) {
	if(obs_scene_ptr == NULL) {
		trace_error("obs_scene_ptr is null");
		return grpc::Status(grpc::INTERNAL, "obs_scene_ptr is null");
	}

	obs_sceneitem_t* si = obs_scene_find_source(*obs_scene_ptr, obs_source_get_name(source));
	if(si == NULL) {
		trace_error("rescue not found in scene");
		return grpc::Status(grpc::INTERNAL, "source not found in scene");
	}

	obs_sceneitem_set_order(si, order);
	return grpc::Status::OK;
}
