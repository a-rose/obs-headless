#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <grpc++/grpc++.h>
#include "lib/proto/studio.grpc.pb.h"
#include "lib/Trace.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using google::protobuf::Empty;

using namespace std;

int gTraceLevel = TRACE_LEVEL_TRACE;
int gTraceFormat = TRACE_FORMAT_TEXT;


class StudioClient {
public:
	StudioClient(shared_ptr<Channel> channel)
		: stub(proto::Studio::NewStub(channel)) {}

	// Studio
	proto::StudioState StudioGet();
	Status StudioStart();
	Status StudioStop();

	// Show
	proto::Show ShowGet(string show_id);
	proto::Show ShowCreate(string show_name);
	proto::Show ShowDuplicate(string show_id);
	Status ShowRemove(string show_id);
	proto::Show ShowLoad(string show_path);

	// Scene
	proto::Scene SceneGet(string show_id, string scene_id);
	proto::Scene SceneAdd(string show_id, string scene_name);
	proto::Scene SceneDuplicate(string show_id, string scene_id);
	Status SceneRemove(string show_id, string scene_id);
	proto::Show SceneSetAsCurrent(string show_id, string scene_id);
	string SceneGetCurrent(string show_id);

	// Source
	proto::Source SourceGet(string show_id, string scene_id, string source_id);
	proto::Source SourceAdd(string show_id, string scene_id, string source_name, string source_type, string source_url);
	proto::Source SourceDuplicate(string show_id, string scene_id, string source_id);
	Status SourceRemove(string show_id, string scene_id, string source_id);
	proto::Source SourceSetProperties(string show_id, string scene_id, string source_id, string source_type, string source_url);

	// Misc
	long Health();

private:
	unique_ptr<proto::Studio::Stub> stub;
};

void switch_scene(StudioClient& client);
void describe_state(StudioClient& client);


int main(int argc, char** argv) {

	try {
		proto::StudioState studio_state;
		Status s;

		// The channel isn't authenticated (use of InsecureChannelCredentials()).
		// TODO URL to settings
		StudioClient client(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));

		long server_timestamp = client.Health();
		if(server_timestamp < 0) {
			throw runtime_error("Failed to get server Health");
		}
		trace_info("Health reply", field(server_timestamp));

		// TODO param
		string show_path = "/etc/shows/bigshow.json";
		client.ShowLoad(OBS_HEADLESS_PATH + show_path);

		trace_info("Starting studio with show", field_ns("show", show_path.c_str()));
		s = client.StudioStart();
        if(!s.ok()) {
            throw runtime_error("Failed to start studio: " + s.error_message());
        }
        
        char c = 0;
        // Wait for 'q' to stop the thread
        do {
            switch(c) {
                case 'd':
                    describe_state(client);
                    break;

                case 's':
                    switch_scene(client);
                    break;

                default:
                    trace_info("----------------------------------------");
                    trace_info("Press 'd' to describe current state");
                    trace_info("Press 's' to switch scene");
                    trace_info("Press 'q' to stop");
            }
            
            c = cin.get();
        } while (c != 'q');

        s = client.StudioStop();
        if(!s.ok()) {
            throw runtime_error("Failed to stop studio: " + s.error_message());
        }

	} catch(const exception& e) {
        trace_error("An exception occured: ", field_ns("exception", e.what()));
    }
    catch(const string& e) {
        trace_error("An exception occured: ", field_ns("exception", e.c_str()));
    }
    catch(...) {
        trace_error("An uncaught exception occured !");
    }

    trace("Exit client");
	return 0;
}


void describe_state(StudioClient& client) {
	proto::StudioState studio_state = client.StudioGet();
	trace_info("Listing studio state (* = active)");

	for(auto show : studio_state.shows()) {
		string show_pre = (show.id() == studio_state.active_show_id()) ? "*" : "-";
		trace_info("  " + show_pre +" Show", field_ns("id", show.id()), field_ns("name", show.name()));

		for(auto scene : show.scenes()) {
			string scene_pre = (scene.id() == show.active_scene_id()) ? "*" : "-";
			trace_info("      " + scene_pre +" Scene", field_ns("id", scene.id()), field_ns("name", scene.name()));

			std::vector<std::string> active_source_ids;
			for(int i=0; i<scene.active_source_ids_size(); i++) {
				active_source_ids.push_back(scene.active_source_ids(i));
			}

			for(auto source : scene.sources()) {
				string source_pre = "-";
				if(std::find(active_source_ids.begin(), active_source_ids.end(), source.id()) != active_source_ids.end()) {
					source_pre = "*";
				}

				trace_info("          " + source_pre +" Source", field_ns("id", source.id()), field_ns("name", source.name()));
			}
		}
	}
}

void switch_scene(StudioClient& client) {
	proto::StudioState studio_state = client.StudioGet();
	string active_show_id = studio_state.active_show_id();
	string active_scene_id = client.SceneGetCurrent(active_show_id);
	string next_scene_id = (active_scene_id == "scene_0") ? "scene_1" : "scene_0";
	trace_info("Switching scene", field_s(active_scene_id), field_s(next_scene_id));
	
	// TODO what do to with returned show ?
	client.SceneSetAsCurrent(active_show_id, next_scene_id);
}

///////////////////
// STUDIO        //
///////////////////

proto::StudioState StudioClient::StudioGet() {
	ClientContext context;
	Empty request;
	proto::StudioGetResponse response;

	Status s = stub->StudioGet(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("StudioGet failed: " + s.error_message());
	}
	return response.studio();
}

Status StudioClient::StudioStart() {
	ClientContext context;
	Empty request, response;

	return stub->StudioStart(&context, request, &response);
}


Status StudioClient::StudioStop() {
	ClientContext context;
	Empty request, response;

	return stub->StudioStop(&context, request, &response);
}

///////////////////
// SHOW          //
///////////////////

proto::Show StudioClient::ShowGet(string show_id) {
	ClientContext context;
	proto::ShowGetRequest request;
	proto::ShowGetResponse response;

	request.set_show_id(show_id);
	Status s = stub->ShowGet(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("ShowGet failed: " + s.error_message());
	}
	return response.show();
}

proto::Show StudioClient::ShowCreate(string show_name) {
	ClientContext context;
	proto::ShowCreateRequest request;
	proto::ShowCreateResponse response;

	request.set_show_name(show_name);
	Status s = stub->ShowCreate(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("ShowCreate failed: " + s.error_message());
	}
	return response.show();
}

proto::Show StudioClient::ShowDuplicate(string show_id) {
	ClientContext context;
	proto::ShowDuplicateRequest request;
	proto::ShowDuplicateResponse response;

	request.set_show_id(show_id);
	Status s = stub->ShowDuplicate(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("ShowDuplicate failed: " + s.error_message());
	}
	return response.show();
}

Status StudioClient::ShowRemove(string show_id) {
	ClientContext context;
	proto::ShowRemoveRequest request;
	Empty response;

	request.set_show_id(show_id);
	return stub->ShowRemove(&context, request, &response);
}

proto::Show StudioClient::ShowLoad(string show_path) {
	ClientContext context;
	proto::ShowLoadRequest request;
	proto::ShowLoadResponse response;

	request.set_show_path(show_path);
	Status s = stub->ShowLoad(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("ShowLoad failed: " + s.error_message());
	}
	return response.show();
}

///////////////////
// SCENE         //
///////////////////

proto::Scene StudioClient::SceneGet(string show_id, string scene_id) {
	ClientContext context;
	proto::SceneGetRequest request;
	proto::SceneGetResponse response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	Status s = stub->SceneGet(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SceneGet failed: " + s.error_message());
	}
	return response.scene();
}

proto::Scene StudioClient::SceneAdd(string show_id, string scene_name) {
	ClientContext context;
	proto::SceneAddRequest request;
	proto::SceneAddResponse response;

	request.set_show_id(show_id);
	request.set_scene_name(scene_name);
	Status s = stub->SceneAdd(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SceneAdd failed: " + s.error_message());
	}
	return response.scene();
}

proto::Scene StudioClient::SceneDuplicate(string show_id, string scene_id) {
	ClientContext context;
	proto::SceneDuplicateRequest request;
	proto::SceneDuplicateResponse response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	Status s = stub->SceneDuplicate(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SceneDuplicate failed: " + s.error_message());
	}
	return response.scene();
}

Status StudioClient::SceneRemove(string show_id, string scene_id) {
	ClientContext context;
	proto::SceneRemoveRequest request;
	Empty response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	return stub->SceneRemove(&context, request, &response);
}

proto::Show StudioClient::SceneSetAsCurrent(string show_id, string scene_id) {
	ClientContext context;
	proto::SceneSetAsCurrentRequest request;
	proto::SceneSetAsCurrentResponse response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	Status s = stub->SceneSetAsCurrent(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SceneSetAsCurrent failed: " + s.error_message());
	}
	return response.show();
}

string StudioClient::SceneGetCurrent(string show_id) {
	ClientContext context;
	proto::SceneGetCurrentRequest request;
	proto::SceneGetCurrentResponse response;

	request.set_show_id(show_id);
	Status s = stub->SceneGetCurrent(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SceneGetCurrent failed: " + s.error_message());
	}
	return response.scene_id();
}

///////////////////
// SOURCE        //
///////////////////

proto::Source StudioClient::SourceGet(string show_id, string scene_id, string source_id) {
	ClientContext context;
	proto::SourceGetRequest request;
	proto::SourceGetResponse response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	request.set_source_id(source_id);
	Status s = stub->SourceGet(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SourceGet failed: " + s.error_message());
	}
	return response.source();
}

proto::Source StudioClient::SourceAdd(string show_id, string scene_id, string source_name, string source_type, string source_url)  {
	ClientContext context;
	proto::SourceAddRequest request;
	proto::SourceAddResponse response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	request.set_source_name(source_name);
	request.set_source_type(source_type);
	request.set_source_url(source_url);
	Status s = stub->SourceAdd(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SourceAdd failed: " + s.error_message());
	}
	return response.source();
}

proto::Source StudioClient::SourceDuplicate(string show_id, string scene_id, string source_id)  {
	ClientContext context;
	proto::SourceDuplicateRequest request;
	proto::SourceDuplicateResponse response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	request.set_source_id(source_id);
	Status s = stub->SourceDuplicate(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SourceDuplicate failed: " + s.error_message());
	}
	return response.source();
}

Status StudioClient::SourceRemove(string show_id, string scene_id, string source_id)  {
	ClientContext context;
	proto::SourceRemoveRequest request;
	Empty response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	request.set_source_id(source_id);
	return stub->SourceRemove(&context, request, &response);
}

proto::Source StudioClient::SourceSetProperties(string show_id, string scene_id, string source_id, string source_type, string source_url) {
	ClientContext context;
	proto::SourceSetPropertiesRequest request;
	proto::SourceSetPropertiesResponse response;

	request.set_show_id(show_id);
	request.set_scene_id(scene_id);
	request.set_source_id(source_id);
	request.set_source_type(source_type);
	request.set_source_url(source_url);
	Status s = stub->SourceSetProperties(&context, request, &response);
	if(!s.ok()) {
		throw runtime_error("SourceSetProperties failed: " + s.error_message());
	}
	return response.source();
}

///////////////////
// MISC          //
///////////////////

long StudioClient::Health() {
	ClientContext context;
	Empty request;
	proto::HealthResponse response;

	Status s = stub->Health(&context, request, &response);
	if(!s.ok()) {
		trace_error("Health failed: " + s.error_message());
		throw runtime_error("Health failed: " + s.error_message());
	}
	return response.timestamp();
}