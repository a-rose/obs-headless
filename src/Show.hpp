#pragma once

#include <jansson.h>
#include "Scene.hpp"

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
	grpc::Status Load(json_t* json_show);
	grpc::Status Start();
	grpc::Status Stop();
	Scene* GetScene(std::string scene_id);
	Scene* AddScene(std::string scene_name);
	Scene* DuplicateSceneFromShow(Show* show, std::string scene_id);
	Scene* DuplicateScene(std::string scene_id);
	grpc::Status RemoveScene(std::string scene_id);
	grpc::Status SwitchScene(std::string scene_id);
	grpc::Status UpdateProto(proto::Show* proto_show);

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
