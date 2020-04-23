#pragma once

#include "Scene.hpp"


enum ShowStatusCode {
	SHOW_OK,
    SHOW_ALREADY_STARTED,
    SHOW_ALREADY_STOPPED,
    SHOW_NOT_FOUND,
    SHOW_SCENE_NOT_FOUND,
    SHOW_SOURCE_NOT_FOUND,
    SHOW_SCENE_ACTIVE,
    SHOW_LIBOBS_ERROR,
};


class ShowStatus : public Status {
public:
	ShowStatus(ShowStatusCode code, std::string message) {
		codeToStr[SHOW_OK] = "OK";
		codeToStr[SHOW_ALREADY_STARTED] = "Show already started";
		codeToStr[SHOW_ALREADY_STOPPED] = "Show already stopped";
		codeToStr[SHOW_NOT_FOUND] = "Show not found";
		codeToStr[SHOW_SCENE_NOT_FOUND] = "Scene not found";
		codeToStr[SHOW_SOURCE_NOT_FOUND] = "Source not found";
		codeToStr[SHOW_SCENE_ACTIVE] = "Scene is active";
		codeToStr[SHOW_LIBOBS_ERROR] = "libobs error";
	}

	ShowStatus(ShowStatusCode code)
		: ShowStatus(code, "") {
	}

	ShowStatus()
		: ShowStatus(SHOW_OK) {
	}
};


///////////////////////////////////////
///////////////////////////////////////


class Show {
public:
	Show(std::string id, std::string name, Settings* settings);
	~Show();

	// Getters
	std::string Id() { return id; }
	std::string Name() { return name; }
	SceneMap Scenes() { return scenes; }
	Scene* ActiveScene() { return active_scene; }
	obs_source_t* Transition() { return obs_transition; }

	// Methods
	ShowStatus Load(json_t* json_show);
	ShowStatus Start();
	ShowStatus Stop();
	Scene* GetScene(std::string scene_id);
	Scene* AddScene(std::string scene_name);
	Scene* DuplicateSceneFromShow(Show* show, std::string scene_id);
	Scene* DuplicateScene(std::string scene_id);
	ShowStatus RemoveScene(std::string scene_id);
	ShowStatus SwitchScene(std::string scene_id);

private:
	std::string id;
	std::string name;
	bool started;
	SceneMap scenes;
	Scene* active_scene;
	obs_source_t* obs_transition;
	Settings* settings;
	uint64_t scene_id_counter;
};

typedef std::map<std::string, Show*> ShowMap;
