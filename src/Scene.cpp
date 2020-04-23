#include "Scene.hpp"

Scene::Scene(std::string id, std::string name, Settings* settings)
	: id(id)
	, name(name)
	, started(false)
	, obs_scene(nullptr)
	, settings(settings)
	, source_id_counter(0)
	, active_source(nullptr) {
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


Source* Scene::AddSource(std::string source_name, SourceType type, std::string source_url) {
	std::string source_id = "source_"+ std::to_string(source_id_counter);
	source_id_counter++;

	Source* source = new Source(source_id, source_name, type, source_url, settings);
	if(!source) {
		trace_error("Failed to create a source", field_s(source_id));
		return NULL;
	}

	trace_debug("Add source", field_s(source_id));
	sources[source_id] = source;
	if(!active_source) {
		active_source = source;// TODO need a setActive method
	}
	return source;
}

Source* Scene::DuplicateSourceFromScene(Scene* scene, std::string source_id) {
	Source* source = scene->GetSource(source_id);
	if(!source) {
		trace_error("Source not found", field_s(source_id));
		return NULL;
	}

	Source* new_source = AddSource(source->Name(), source->Type(), source->Url());
	if(!new_source) {
		trace_error("Failed to duplicate source");
		return NULL;
	}

	return new_source;
}

Source* Scene::DuplicateSource(std::string source_id) {
	return DuplicateSourceFromScene(this, source_id);
}

SceneStatus Scene::RemoveSource(std::string source_id) {
	SourceMap::iterator it = sources.find(source_id);
	if(it == sources.end()) {
		trace_error("Source not found", field_s(source_id));
		return SceneStatus(SCENE_SOURCE_NOT_FOUND, source_id);
	}
	if(it->second == active_source) {
		trace_error("Source is active", field_s(source_id));
		return SceneStatus(SCENE_SOURCE_ACTIVE, source_id);
	}

	trace_debug("Remove source", field_s(source_id));
	// No need to do source->Stop(); because it is not actve
	delete it->second;
	sources.erase(it);

	return SceneStatus(SCENE_OK);
}

SceneStatus Scene::Start() {
	trace_debug("Start scene", field_s(id));

	if(started) {
		trace_error("Scene already started", field_s(id));
		return SceneStatus(SCENE_ALREADY_STARTED, id);
	}


	// scene (contains the source)
	std::string scene_name = std::string("obs_scene_"+ id);
	obs_scene = obs_scene_create(scene_name.c_str());
	if (!obs_scene) {
		trace_error("Error while creating obs_scene", field_s(id));
		return SceneStatus(SCENE_LIBOBS_ERROR, "Error while creating obs_scene");
	}


	SourceStatus s = active_source->Start(&obs_scene);
	if(!s.ok()) {
		trace_error("source Start failed", error(s.error_message()));
		return SceneStatus(SCENE_SOURCE_NOT_FOUND, s.error_message());
	}

	started = true;
	return SceneStatus(SCENE_OK);
}


SceneStatus Scene::Stop() {
	trace_debug("Stop scene", field_s(id));

	if(!started) {
		trace_error("Scene already stopped", field_s(id));
		return SceneStatus(SCENE_ALREADY_STOPPED, id);
	}

	SourceStatus s = active_source->Stop();
	if(!s.ok()) {
		trace_error("Source Stop failed", field_s(id), error(s.error_message()));
		return SceneStatus(SCENE_SOURCE_NOT_FOUND, s.error_message());
	}

	obs_scene_release(obs_scene);
	started = false;

	return SceneStatus(SCENE_OK);
}
