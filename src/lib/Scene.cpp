#include <algorithm>
#include "Scene.hpp"

Scene::Scene(std::string id, std::string name, Settings* settings)
	: id(id)
	, name(name)
	, started(false)
	, obs_scene(nullptr)
	, settings(settings)
	, source_id_counter(0) {
	trace_debug("Create Scene", field_s(id), field_s(name));
}

Scene::~Scene() {
	SourceMap::iterator it;
	for (it = sources.begin(); it != sources.end(); it++) {
		delete it->second;
	}
}

Source* Scene::GetSource(std::string source_id) {
	SourceMap::iterator it = sources.find(source_id);
	if (it == sources.end()) {
		return NULL;
	}
	return it->second;
}


Source* Scene::AddSource(std::string source_name, SourceType type, std::string source_url, int width, int height) {
	std::string source_id = "source_"+ std::to_string(source_id_counter);
	source_id_counter++;

	Source* source = new Source(source_id, source_name, type, source_url, width, height, settings);
	if(!source) {
		trace_error("Failed to create a source", field_s(source_id));
		return NULL;
	}

	trace_debug("Add source", field_s(source_id));
	sources[source_id] = source;

	// TODO at the moment, all sources are always active. Add a way to switch
	// sources on and off.
	active_sources.push_back(source); // TODO need a setActive method
	return source;
}

Source* Scene::DuplicateSourceFromScene(Scene* scene, std::string source_id) {
	Source* source = scene->GetSource(source_id);
	if(!source) {
		trace_error("Source not found", field_s(source_id));
		return NULL;
	}

	// TODO width & height
	Source* new_source = AddSource(source->Name(), source->Type(), source->Url(), -1, -1);
	if(!new_source) {
		trace_error("Failed to duplicate source");
		return NULL;
	}

	return new_source;
}

Source* Scene::DuplicateSource(std::string source_id) {
	return DuplicateSourceFromScene(this, source_id);
}

grpc::Status Scene::RemoveSource(std::string source_id) {
	SourceMap::iterator it = sources.find(source_id);
	if(it == sources.end()) {
		trace_error("Source not found", field_s(source_id));
		return grpc::Status(grpc::NOT_FOUND, "Source not found id="+ source_id);
	}

	if(std::find(active_sources.begin(), active_sources.end(), it->second) != active_sources.end()) {
		trace_error("Source is active", field_s(source_id));
		return grpc::Status(grpc::FAILED_PRECONDITION, "Source is active id="+ source_id);
	}

	trace_debug("Remove source", field_s(source_id));
	// No need to do source->Stop(); because it is not actve
	delete it->second;
	sources.erase(it);

	return grpc::Status::OK;
}

grpc::Status Scene::Start() {
	grpc::Status s;
	trace_debug("Start scene", field_s(id));

	if(started) {
		trace_error("Scene already started", field_s(id));
		return grpc::Status(grpc::FAILED_PRECONDITION, "Scene already started");
	}


	// scene (contains the source)
	std::string scene_name = std::string("obs_scene_"+ id);
	obs_scene = obs_scene_create(scene_name.c_str());
	if (!obs_scene) {
		trace_error("Error while creating obs_scene", field_s(id));
		return grpc::Status(grpc::INTERNAL, "Error while creating obs_scene");
	}


	for (auto & source : active_sources) {
		s = source->Start(&obs_scene);
		if(!s.ok()) {
			trace_error("source Start failed", field_s(source->Id()), error(s.error_message()));
			return s;
		}
	}

	started = true;
	return grpc::Status::OK;
}


grpc::Status Scene::Stop() {
	grpc::Status s;
	trace_debug("Stop scene", field_s(id));

	if(!started) {
		trace_error("Scene already stopped", field_s(id));
		return grpc::Status(grpc::FAILED_PRECONDITION, "Scene already stopped");
	}

	for (auto & source : active_sources) {
		s = source->Stop();
		if(!s.ok()) {
			trace_error("Source Stop failed", field_s(source->Id()), error(s.error_message()));
			return s;
		}
	}

	obs_scene_release(obs_scene);
	started = false;

	return grpc::Status::OK;
}

grpc::Status Scene::UpdateProto(proto::Scene* proto_scene) {
	proto_scene->Clear();
	proto_scene->set_id(id);
	proto_scene->set_name(name);

	for (auto & s : active_sources) {
		proto_scene->add_active_source_ids(s->Id());
	}

	SourceMap::iterator it;
	for (it = sources.begin(); it != sources.end(); it++) {
		proto::Source* proto_source = proto_scene->add_sources();
		Source* source = it->second;

		grpc::Status s = source->UpdateProto(proto_source);
		if(!s.ok()) {
			trace_error("failed to update source proto");
			return s;
		}
	}

	return grpc::Status::OK;
}
