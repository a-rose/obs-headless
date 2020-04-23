#include "Show.hpp"

Show::Show(std::string id, std::string name, Settings* settings)
	: id(id)
	, name(name)
	, started(false)
	, settings(settings)
	, obs_transition(nullptr)
	, active_scene(nullptr)
	, scene_id_counter(0) {
	trace_debug("Create Show", field_s(id), field_s(name));
}

Show::~Show() {
	SceneMap::iterator it;
	for (it = scenes.begin(); it != scenes.end(); it++) {
		delete it->second;
	}
}

ShowStatus Show::Load(json_t* jsonShow) {
	json_t* jsonShowName = json_object_get(jsonShow, "name");

	if(!jsonShowName) {
		trace_error("Show name not found in json", field_s(id));
		return ShowStatus(SHOW_NOT_FOUND, id);
	}

	const char* strShowName = json_string_value(jsonShowName);
	if(!strShowName) {
		trace_error("Can't read show name", field_s(id));
		return ShowStatus(SHOW_NOT_FOUND, "Can't read show name: " + id);
	}
	name = std::string(strShowName);
	trace_debug("Update show name", field_s(name));

	json_t* jsonScenes = json_object_get(jsonShow, "scenes");
	if(!jsonScenes) {
		trace_error("Scenes not found in json", field_s(id));
		return ShowStatus(SHOW_SCENE_NOT_FOUND, "scenes not found in config");
	}

	size_t sceneIdx;
	json_t *jsonScene;

	json_array_foreach(jsonScenes, sceneIdx, jsonScene) {
		json_t* jsonSceneName = json_object_get(jsonScene, "name");
		if(!jsonSceneName) {
			trace_error("Scene name not found in json", field(sceneIdx));
			return ShowStatus(SHOW_SCENE_NOT_FOUND, "Scene name not found in config file sceneIdx="+ std::to_string(sceneIdx));
		}

		const char* strSceneName = json_string_value(jsonSceneName);
		if(!strSceneName) {
			trace_error("Can't read scene name", field(sceneIdx));
			return ShowStatus(SHOW_SCENE_NOT_FOUND, "Can't read scene name sceneIdx="+ std::to_string(sceneIdx));
		}

		Scene* scene = AddScene(std::string(strSceneName));
		if(!scene) {
			trace_error("Failed to add scene", field(sceneIdx), field_c(strSceneName));
			return ShowStatus(SHOW_LIBOBS_ERROR, "Failed to add scene sceneIdx="+ std::to_string(sceneIdx));
		}

		json_t* jsonSources = json_object_get(jsonScene, "sources");
		if(!jsonSources) {
			trace_error("Sources not found in json", field(sceneIdx));
			return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Sources not found in config file sceneIdx="+ std::to_string(sceneIdx));
		}

		size_t sourceIdx;
		json_t* jsonSource;

		json_array_foreach(jsonSources, sourceIdx, jsonSource) {
			json_t* jsonSourceName = json_object_get(jsonSource, "name");
			if(!jsonSourceName) {
				trace_error("Source name not found in json", field(sceneIdx), field(sourceIdx));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Source name not found in config file sceneIdx="+ std::to_string(sceneIdx) +", sourceIdx="+ std::to_string(sourceIdx));
			}

			const char* strSourceName = json_string_value(jsonSourceName);
			if(!strSourceName) {
				trace_error("Can't read source name", field(sceneIdx), field(sourceIdx));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Can't read source name sceneIdx="+ std::to_string(sceneIdx) +", sourceIdx="+ std::to_string(sourceIdx));
			}

			json_t* jsonSourceUrl = json_object_get(jsonSource, "url");
			if(!jsonSourceUrl) {
				trace_error("Source url not found in json", field(sceneIdx), field(sourceIdx));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Source url not found in config file sceneIdx="+ std::to_string(sceneIdx) +", sourceIdx="+ std::to_string(sourceIdx));
			}

			const char* strSourceUrl = json_string_value(jsonSourceUrl);
			if(!strSourceUrl) {
				trace_error("Can't read source url", field(sceneIdx), field(sourceIdx));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Can't read source url sceneIdx="+ std::to_string(sceneIdx) +", sourceIdx="+ std::to_string(sourceIdx));
			}

			json_t* jsonSourceType = json_object_get(jsonSource, "type");
			if(!jsonSourceType) {
				trace_error("Source type not found in json", field(sceneIdx), field(sourceIdx));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Source type not found in config file sceneIdx="+ std::to_string(sceneIdx) +", sourceIdx="+ std::to_string(sourceIdx));
			}

			const char* strSourceType = json_string_value(jsonSourceType);
			if(!strSourceType) {
				trace_error("Can't read source type", field(sceneIdx), field(sourceIdx));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Can't read source type sceneIdx="+ std::to_string(sceneIdx) +", sourceIdx="+ std::to_string(sourceIdx));
			}

			SourceType type = StringToSourceType(std::string(strSourceType));
			if(type == InvalidType) {
				trace_error("Unsupported source type", field_c(strSourceType));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Unsupported source type="+ std::string(strSourceType));
			}

			Source* source = scene->AddSource(std::string(strSourceName), type, std::string(strSourceUrl));
			if(!source) {
				trace_error("Failed to add source", field(sceneIdx), field(sourceIdx));
				return ShowStatus(SHOW_SOURCE_NOT_FOUND, "Failed to add source sceneIdx="+ std::to_string(sceneIdx) +", sourceIdx="+ std::to_string(sourceIdx));
			}
		}
	}

	return ShowStatus(SHOW_OK);
}

ShowStatus Show::Start() {
	if(started) {
		trace_error("Show already started", field_s(id));
		return ShowStatus(SHOW_ALREADY_STARTED, id);
	}

	SceneStatus s = active_scene->Start();
	if(!s.ok()) {
		trace_error("Scene Start failed", error(s.error_message()));
		return ShowStatus(SHOW_SCENE_NOT_FOUND, s.error_message());
	}

	// transition (contains the scene)
	std::string transition_name = std::string("transition_"+ name);
	obs_transition = obs_source_create(settings->transition_type.c_str(), transition_name.c_str(), NULL, nullptr);
	if (!obs_transition) {
		trace_error("Error while creating obs_transition", field_s(id));
		return ShowStatus(SHOW_LIBOBS_ERROR, "Error while creating obs_transition");
	}
	obs_transition_set(obs_transition, obs_scene_get_source(active_scene->GetScene()));

	started = true;
	return ShowStatus(SHOW_OK);
}

ShowStatus Show::Stop() {
	if(!started) {
		trace_error("Show already stopped", field_s(id));
		return ShowStatus(SHOW_ALREADY_STOPPED, id);
	}

	SceneStatus s = active_scene->Stop();
	if(!s.ok()) {
		trace_error("Scene Stop failed", error(s.error_message()));
		return ShowStatus(SHOW_SCENE_NOT_FOUND, s.error_message());
	}

	trace_debug("clear and release obs_transition");
	obs_transition_clear(obs_transition);
	obs_source_release(obs_transition);

	started = false;
	return ShowStatus(SHOW_OK);
}

Scene* Show::GetScene(std::string scene_id) {
	SceneMap::iterator it = scenes.find(scene_id);
	if (it == scenes.end()) {
		return NULL;
	}
	return it->second;
}

Scene* Show::AddScene(std::string scene_name) {
	std::string scene_id = "scene_"+ std::to_string(scene_id_counter);
	scene_id_counter++;

	Scene* scene = new Scene(scene_id, scene_name, settings);
	if(!scene) {
		trace_error("Failed to create a scene", field_s(scene_id));
		return NULL;
	}

	trace_debug("Add scene", field_s(scene_id));
	scenes[scene_id] = scene;
	if(!active_scene) {
		active_scene = scene;// TODO need a setActive method
	}
	return scene;
}

Scene* Show::DuplicateSceneFromShow(Show* show, std::string scene_id) {
	Scene* scene = show->GetScene(scene_id);
	if(!scene) {
		trace_error("Scene not found", field_s(scene_id));
		return NULL;
	}

	Scene* new_scene = AddScene(scene->Name());
	if(!new_scene) {
		trace_error("Failed to duplicate scene");
		return NULL;
	}

	SourceMap::iterator it;
	for (it = scene->Sources().begin(); it != scene->Sources().end(); it++) {
		Source* source = it->second;
		trace_debug("source from original scene",
			field_ns("id", source->Id()),
			field_ns("name", source->Name()),
			field_ns("url", source->Url()));

		Source* new_source = new_scene->DuplicateSourceFromScene(scene, source->Id());
		if(!new_source) {
			trace_error("Failed to duplicate source");
			return NULL;
		}
	}

	return new_scene;
}

Scene* Show::DuplicateScene(std::string scene_id) {
	return DuplicateSceneFromShow(this, scene_id);
}

ShowStatus Show::RemoveScene(std::string scene_id) {
	SceneMap::iterator it = scenes.find(scene_id);
	if(it == scenes.end()) {
		trace_error("Scene not found", field_s(scene_id));
		return ShowStatus(SHOW_SCENE_NOT_FOUND, "Scene not found id="+ scene_id);
	}
	if(it->second == active_scene) {
		trace_error("Scene is active", field_s(scene_id));
		return ShowStatus(SHOW_SCENE_ACTIVE, "id="+ scene_id);
	}

	trace_debug("Remove scene", field_s(scene_id));
	// No need to do scene->Stop(); because it is not actve
	delete it->second;
	scenes.erase(it);

	return ShowStatus(SHOW_OK);
}

ShowStatus Show::SwitchScene(std::string scene_id) {
	Scene* next = GetScene(scene_id);
	Scene* curr = active_scene;

	if(!next) {
		trace_error("Scene not found", field_s(scene_id));
		return ShowStatus(SHOW_SCENE_NOT_FOUND, scene_id);
	}
	if(next == active_scene) {
		trace_error("Scene already active", field_s(scene_id));
		return ShowStatus(SHOW_SCENE_ACTIVE, scene_id);
	}

	SceneStatus s = next->Start();
	if(!s.ok()) {
		trace_error("Scene Start failed", error(s.error_message()));
		return ShowStatus(SHOW_SCENE_NOT_FOUND, s.error_message());
	}

	trace_debug("start transition");
	bool ret = obs_transition_start(
		obs_transition,
		OBS_TRANSITION_MODE_AUTO,
		settings->transition_duration_ms,
		obs_scene_get_source(next->GetScene())
	);

	if(ret != true) {
		trace_error("obs_transition_start failed", field_s(id));
		return ShowStatus(SHOW_LIBOBS_ERROR, "obs_transition_start failed");
	}

	trace_debug("transition finished");
	Scene* prev = active_scene;
	active_scene = next;

	s = prev->Stop();
	if(!s.ok()) {
		trace_error("Scene Stop failed", error(s.error_message()));
		return ShowStatus(SHOW_SCENE_NOT_FOUND, s.error_message());
	}

	return ShowStatus(SHOW_OK);
}
