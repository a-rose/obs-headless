#pragma once

#include <vector>
#include "Source.hpp"

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
	Source* AddSource(std::string source_name, SourceType type, std::string source_url, int width, int height);
	Source* DuplicateSourceFromScene(Scene* scene, std::string source_id);
	Source* DuplicateSource(std::string source_id);
	grpc::Status RemoveSource(std::string source_id);
	grpc::Status Start();
	grpc::Status Stop();
	grpc::Status UpdateProto(proto::Scene* proto_scene);

private:
	std::string id;
	std::string name;
	bool started;
	obs_scene_t* obs_scene;
	SourceMap sources;
	std::vector<Source*> active_sources;
	Settings* settings;
	uint64_t source_id_counter;
};

typedef std::map<std::string, Scene*> SceneMap;
