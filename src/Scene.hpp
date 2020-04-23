#pragma once

#include "Source.hpp"


enum SceneStatusCode {
	SCENE_OK,
    SCENE_ALREADY_STARTED,
    SCENE_ALREADY_STOPPED,
	SCENE_SOURCE_NOT_FOUND,
    SCENE_SOURCE_ACTIVE,
    SCENE_LIBOBS_ERROR,
};

class SceneStatus : public Status {
public:
	SceneStatus(SceneStatusCode code, std::string message) {
		codeToStr[SCENE_OK] = "OK";
		codeToStr[SCENE_ALREADY_STARTED] = "Scene already started";
		codeToStr[SCENE_ALREADY_STOPPED] = "Scene already stopped";
		codeToStr[SCENE_SOURCE_NOT_FOUND] = "Source not found";
		codeToStr[SCENE_SOURCE_ACTIVE] = "Source is active";
		codeToStr[SCENE_LIBOBS_ERROR] = "libobs error";
	}

	SceneStatus(SceneStatusCode code)
		: SceneStatus(code, "") {
	}

	SceneStatus()
		: SceneStatus(SCENE_OK) {
	}
};


///////////////////////////////////////
///////////////////////////////////////


class Scene {
public:
	Scene(std::string id, std::string name, Settings* settings);
	~Scene();

	// Getters
	std::string Id() { return id; }
	std::string Name() { return name; }
	SourceMap Sources() { return sources; }
	obs_scene_t* GetScene() { return obs_scene; }

	// Methods
	Source* GetSource(std::string source_id);
	Source* AddSource(std::string source_name, SourceType type, std::string source_url);
	Source* DuplicateSourceFromScene(Scene* scene, std::string source_id);
	Source* DuplicateSource(std::string source_id);
	SceneStatus RemoveSource(std::string source_id);
	SceneStatus Start();
	SceneStatus Stop();

private:
	std::string id;
	std::string name;
	bool started;
	obs_scene_t* obs_scene;
	SourceMap sources;
	Source* active_source;
	Settings* settings;
	uint64_t source_id_counter;
};

typedef std::map<std::string, Scene*> SceneMap;
