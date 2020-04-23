#pragma once

#include <string>
#include "obs.h"
#include "Trace.hpp"
#include "Settings.hpp"
#include "Status.hpp"

#if 0
#define STATE_STR(state) ( \
		state == OBS_STREAM_IDLE ? "idle" \
	:	state == OBS_STREAM_ABSENT ? "absent" \
	:	state == OBS_STREAM_ALIVE ? "alive" \
	: "timeout" \
)
#else
// TODO reimplement
#define STATE_STR(state) ("unknown")
#endif

enum SourceStatusCode {
	SOURCE_OK,
	SOURCE_ALREADY_STARTED,
    SOURCE_ALREADY_STOPPED,
    SOURCE_INVALID_TYPE,
    SOURCE_LIBOBS_ERROR,
};


class SourceStatus : public Status {
public:
	SourceStatus(SourceStatusCode code, std::string message) {
		codeToStr[SOURCE_OK] = "OK";
		codeToStr[SOURCE_ALREADY_STARTED] = "Source already started";
		codeToStr[SOURCE_ALREADY_STOPPED] = "Source already stopped";
		codeToStr[SOURCE_INVALID_TYPE] = "Invalid type";
		codeToStr[SOURCE_LIBOBS_ERROR] = "libobs error";
	}

	SourceStatus(SourceStatusCode code)
		: SourceStatus(code, "") {
	}

	SourceStatus()
		: SourceStatus(SOURCE_OK) {
	}
};


///////////////////////////////////////
///////////////////////////////////////


enum SourceType {
	InvalidType = -1,
	Image = 0,
	RTMP
};

std::string SourceTypeToString(SourceType type);
SourceType StringToSourceType(std::string type);


class Source {
public:
	Source(std::string id, std::string name, SourceType type, std::string url, Settings* settings);
	~Source();

	// Getters
	std::string Id() { return id; }
	std::string Name() { return name; }
	SourceType Type() { return type; }
	std::string Url() { return url; }
	obs_source_t* GetSource() { return obs_source; }

	// Methods
	SourceStatus SetType(std::string new_type);
	SourceStatus SetUrl(std::string new_url);
	SourceStatus Start(obs_scene_t** obs_scene_ptr);
	SourceStatus Stop();

private:
	SourceStatus addSourceToScene(obs_source_t* source);
	SourceStatus setSourceOrder(obs_source_t* source, enum obs_order_movement order);

	std::string id;
	std::string name;
	SourceType type;
	std::string url;
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
