#pragma once

#include "Show.hpp"
#include <mutex>

/**
 * @file
 * @brief gRPC Server class.
 *
 * The server responds to all methods declared in proto/studio.proto.
 * It can contain Show objects in the `ShowMap shows` map.
 *
 */

using namespace std;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using google::protobuf::Empty;

class Studio final : public proto::Studio::Service {
public:
	/**
	 * Studio constructor.
	 *
	 * @param   settings  Pointer to the application settings. The pointer is
	 *               copied internally.
	 */
	Studio(Settings* settings);

	/**
	 * Studio destructor. Deletes any non-NULL show in `shows`.
	 */
	~Studio();

	// Studio

	/**
	 * Returns the current Studio state (each show, scene and source) to the
	 * gRPC caller.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  Empty request gRPC type.
	 * @param   rep  the studio state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INTERNAL if an exception occured or a show is NULL
	 */
	Status StudioGet(ServerContext* ctx, const Empty* req, proto::StudioGetResponse* rep) override;

	/**
	 * Calls studioInit to start the studio.
	 *
	 * @note a show must be active (for example with ShowLoad)
	 * @note cannot be called twice without calling StudioStop in between.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  Empty request gRPC type.
	 * @param   rep  Empty response gRPC type.
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INTERNAL if an exception occured
	 */
	Status StudioStart(ServerContext* ctx, const Empty* req, Empty* rep) override;

	/**
	 * Calls studioRelease to stop the studio.
	 *
	 * @note cannot be called if StudioStart was not called before.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  Empty request gRPC type.
	 * @param   rep  Empty response gRPC type.
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INTERNAL if an exception occured
	 */
	Status StudioStop(ServerContext* ctx, const Empty* req, Empty* rep) override;

	// Show
	/**
	 * Returns the state of a given show to the gRPC caller.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  ShowGetRequest containing the show_id.
	 * @param   rep  the show state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id is not found in the shows map
	 *               grpc::Status::INTERNAL if an exception occured or a scene is NULL
	 */
	Status ShowGet(ServerContext* ctx, const proto::ShowGetRequest* req, proto::ShowGetResponse* rep) override;

	/**
	 * Creates a new empty show and adds it to the shows map.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  ShowCreateRequest containing the show_name.
	 * @param   rep  the show state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INTERNAL if an exception occured
	 */
	Status ShowCreate(ServerContext* ctx, const proto::ShowCreateRequest* req, proto::ShowCreateResponse* rep) override;

	/**
	 * Creates a new show from an existing one and adds it to the shows map. It
	 * will have the same name with a different id. All scenes and sources are
	 * also duplicated. The new show is not set as active and is not started.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  ShowDuplicateRequest containing the show_id to duplicate.
	 * @param   rep  the show state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INTERNAL if an exception occured or the original show is not found
	 */
	Status ShowDuplicate(ServerContext* ctx, const proto::ShowDuplicateRequest* req, proto::ShowDuplicateResponse* rep) override;

	/**
	 * Calls removeShow to remove a show.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  ShowRemoveRequest containing the show_id to remove.
	 * @param   rep  Empty response gRPC type.
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INTERNAL if an exception occured or the show is not found
	 */
	Status ShowRemove(ServerContext* ctx, const proto::ShowRemoveRequest* req, Empty* rep) override;

	/**
	 * Calls loadShow to load a show.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  ShowLoadRequest containing the path to load the show from
	 *               (named show_id).
	 * @param   rep  the show state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INTERNAL if an exception occured or the show failed to load
	 */
	Status ShowLoad(ServerContext* ctx, const proto::ShowLoadRequest* req, proto::ShowLoadResponse* rep) override;

	// Scene
	/**
	 * Returns the state of a given scene to the gRPC caller.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SceneGetRequest containing the show_id and scene_id.
	 * @param   rep  the scene state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND show_id is not found in the show
	 *               map or if scene_id is not found in show_id.
	 *               grpc::Status::INTERNAL if an exception occured
	 */
	Status SceneGet(ServerContext* ctx, const proto::SceneGetRequest* req, proto::SceneGetResponse* rep) override;

	/**
	 * Creates a new empty scene and adds it to the scenes map of a given show.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SceneAddRequest containing the show_id and new scene_name.
	 * @param   rep  the scene state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND show_id is not found in the show map
	 *               grpc::Status::INTERNAL if an exception occured or failed to
	 *               add scene
	 */
	Status SceneAdd(ServerContext* ctx, const proto::SceneAddRequest* req, proto::SceneAddResponse* rep) override;

	/**
	 * Creates a new scene from an existing one and adds it to the scenes map of
	 * a given show. It will have the same name with a different id. All sources
	 * are also duplicated. The new scene is not set as active and is not started.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SceneDuplicateRequest containing the show_id where the
	 *               scene to duplicate is located, and its scene_id.
	 * @param   rep  the scene state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id is not found
	 *               grpc::Status::INTERNAL if an exception occured or the
	 *               original scene is not found
	 */
	Status SceneDuplicate(ServerContext* ctx, const proto::SceneDuplicateRequest* req, proto::SceneDuplicateResponse* rep) override;

	/**
	 * Removes a given scene from a given show.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SceneRemoveRequest containing the show_id that contains the
	 *               scene_id to remove.
	 * @param   rep  Empty response gRPC type.
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id or scene_id is not found
	 *               grpc::Status::FAILED_PRECONDITION if the scene is active
	 *               grpc::Status::INTERNAL if an exception occured
	 */
	Status SceneRemove(ServerContext* ctx, const proto::SceneRemoveRequest* req, Empty* rep) override;

	/**
	 * Sets a given scene as active in a given show : the show switches to this
	 * scene.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SceneSetAsCurrentRequest containing the show_id that
	 *               contains the sceene, and scene_id of the scene to switch to.
	 * @param   rep  the show state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id or scene_id is not found
	 *               grpc::Status::INVALID_ARGUMENT if the scene is already active
	 *               grpc::Status::INTERNAL if an exception occured or the scene transition failed
	 */
	Status SceneSetAsCurrent(ServerContext* ctx, const proto::SceneSetAsCurrentRequest* req, proto::SceneSetAsCurrentResponse* rep) override;

	/**
	 * Returns the id of the currently active scene t othe gRPC caller.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SceneGetCurrentRequest containing the show_id
	 * @param   rep  SceneGetCurrentResponse containing the active scene_id.
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id is not found
	 *               grpc::Status::INTERNAL if an exception occured or the active scene is NULL
	 */
	Status SceneGetCurrent(ServerContext* ctx, const proto::SceneGetCurrentRequest* req, proto::SceneGetCurrentResponse* rep) override;

	// Source
	/**
	 * Returns the state of a given source to the gRPC caller.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SourceGetRequest containing the show_id, scene_id and source_id
	 * @param   rep  the source state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id is not found in the show
	 *               map or if scene_id is not found in show_id or if source_id
	 *               is not found in scene_id
	 *               grpc::Status::INTERNAL if an exception occured
	 */
	Status SourceGet(ServerContext* ctx, const proto::SourceGetRequest* req, proto::SourceGetResponse* rep) override;

	/**
	 * Creates a new empty source and adds it to the source map of a given scene
	 * in a given show.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SourceAddRequest containing the show_id and scene_id, with
	 *               the new source_name, source_type, and source_url.
	 * @param   rep  the source state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::INVALID_ARGUMENT if the source_type is not supported
	 *               grpc::Status::NOT_FOUND if show_id is not found in the show
	 *               map or if scene_id is not found in show_id
	 *               grpc::Status::INTERNAL if an exception occured or failed to
	 *               add source
	 */
	Status SourceAdd(ServerContext* ctx, const proto::SourceAddRequest* req, proto::SourceAddResponse* rep) override;

	/**
	 * Creates a new source from an existing one and adds it to the source map
	 * of a given scene in a given show. It will have the same name, type and
	 * url with a different id. The new source is not set as active not started.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SourceDuplicateRequest containing the show_id and scene_id
	 *               where the source to duplicate is located, and its source_id.
	 * @param   rep  the source state (see proto/studio.proto).
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id or scene_id is not found
	 *               grpc::Status::INTERNAL if an exception occured or the
	 *               original source is not found
	 */
	Status SourceDuplicate(ServerContext* ctx, const proto::SourceDuplicateRequest* req, proto::SourceDuplicateResponse* rep) override;

	/**
	 * Removes a given source from a given scene in a given show.
	 *
	 * @param   ctx  pointer to the gRPC server context.
	 * @param   req  SourceRemoveRequest containing the show_id and scene_id
	 *               that contain the source_id to remove.
	 * @param   rep  Empty response gRPC type.
	 * @return       grpc::Status::OK if successful
	 *               grpc::Status::NOT_FOUND if show_id, scene_id or source_id is not found
	 *               grpc::Status::FAILED_PRECONDITION if the source is active
	 *               grpc::Status::INTERNAL if an exception occured
	 */
	Status SourceRemove(ServerContext* ctx, const proto::SourceRemoveRequest* req, Empty* rep) override;
	
	// TODO doc
	Status SourceSetProperties(ServerContext* ctx, const proto::SourceSetPropertiesRequest* req, proto::SourceSetPropertiesResponse* rep) override;

	// Misc
	Status Health(ServerContext* ctx, const Empty* req, proto::HealthResponse* rep) override;

private:
	//Initializes obs: reset video and audio context, load modules libs, create RTMP output and encoders. Then starts the currently active show.
	Status studioInit();
	// Releases the obs output and encoders, stop the currently active show, stops obs.
	Status studioRelease();
	Show* getShow(string show_id);
	Show* addShow(string show_name);
	Show* loadShow(string show_id);
	Show* duplicateShow(string show_id);
	Status removeShow(string show_id);
	int loadModule(const char* binPath, const char* dataPath);


	bool init;
	ShowMap shows;
	Show* active_show;

	//show_id_counter is incremented for each created show.
	uint64_t show_id_counter;

	Settings* settings;

	struct obs_video_info ovi;
	struct obs_audio_info oai;

    obs_service_t*  service;
	obs_output_t*   output;
	obs_encoder_t*  enc_a;
	obs_encoder_t*  enc_v;
	obs_data_t*     rtmp_settings;
	obs_data_t*     enc_a_settings;
	obs_data_t*     enc_v_settings;

	std::mutex mtx;
};