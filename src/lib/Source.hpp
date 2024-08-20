#pragma once

#include <string>
#include <grpc++/grpc++.h>
#include "proto/studio.grpc.pb.h"
#include "obs.h"
#include "Trace.hpp"
#include "Settings.hpp"


enum SourceType {
	InvalidType = -1,
	Image = 0,
	RTMP
};

std::string SourceTypeToString(SourceType type);
SourceType StringToSourceType(std::string type);


class Source {
public:
	Source(std::string id, std::string name, SourceType type, std::string url, int width, int height, Settings* settings);
	~Source();

	// Getters
	std::string Id() { return id; }
	std::string Name() { return name; }
	SourceType Type() { return type; }
	std::string Url() { return url; }
	obs_source_t* GetSource() { return obs_source; }

	// Methods
	grpc::Status SetType(std::string new_type);
	grpc::Status SetUrl(std::string new_url);
	grpc::Status Start(obs_scene_t** obs_scene_ptr);
	grpc::Status Stop();
	grpc::Status UpdateProto(proto::Source* proto_source);


private:
	grpc::Status addSourceToScene(obs_source_t* source);
	grpc::Status setSourceOrder(obs_source_t* source, enum obs_order_movement order);

	std::string id;
	std::string name;
	SourceType type;
	std::string url;
	int width;
	int height;
	bool started;
	obs_source_t* obs_source;
	obs_scene_t** obs_scene_ptr;
	Settings* settings;
};

void SourceShowCb(void *my_data, calldata_t *cd);
void SourceHideCb(void *my_data, calldata_t *cd);
void SourceActivateCb(void *my_data, calldata_t *cd);
void SourceTransitionStartCb(void *my_data, calldata_t *cd);
void SourceTransitionVideoStopCb(void *my_data, calldata_t *cd);
void SourceTransitionStopCb(void *my_data, calldata_t *cd);

typedef std::map<std::string, Source*> SourceMap;
