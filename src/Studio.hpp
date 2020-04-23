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


enum StudioStatusCode {
	STUDIO_OK,
    STUDIO_ALREADY_STARTED,
    STUDIO_ALREADY_STOPPED,
    STUDIO_SHOW_NOT_FOUND,
    STUDIO_SCENE_NOT_FOUND,
    STUDIO_SOURCE_NOT_FOUND,
    STUDIO_SHOW_ACTIVE,
    STUDIO_LIBOBS_ERROR,
    STUDIO_ERROR,
};

class StudioStatus : public Status {
public:
	StudioStatus(StudioStatusCode code, std::string message) {
	    codeToStr[STUDIO_OK] = "OK";
	    codeToStr[STUDIO_ALREADY_STARTED] = "Studio already started";
        codeToStr[STUDIO_ALREADY_STOPPED] = "Studio already stopped";
        codeToStr[STUDIO_SHOW_NOT_FOUND] = "Show not found";
        codeToStr[STUDIO_SCENE_NOT_FOUND] = "Scene not found";
        codeToStr[STUDIO_SOURCE_NOT_FOUND] = "Source not found";
        codeToStr[STUDIO_SHOW_ACTIVE] = "Show is active";
        codeToStr[STUDIO_LIBOBS_ERROR] = "libobs error";
        codeToStr[STUDIO_ERROR] = "Error";
	}

	StudioStatus(StudioStatusCode code)
		: StudioStatus(code, "") {
	}

	StudioStatus()
		: StudioStatus(STUDIO_OK) {
	}
};


///////////////////////////////////////
///////////////////////////////////////

class Studio {
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

    // TODO update doc (params, replace all @return's with good doc)

	/**
	 * Returns the current Studio state (each show, scene and source)
     * 
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INTERNAL if an exception occured or a show is NULL
	 */
	StudioStatus GetStudio();

	/**
	 * Calls studioInit to start the studio.
	 *
	 * @note a show must be active (for example with ShowLoad)
	 * @note cannot be called twice without calling StudioStop in between.
     * 
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INTERNAL if an exception occured
	 */
	StudioStatus StudioStart();

	/**
	 * Calls studioRelease to stop the studio.
	 *
	 * @note cannot be called if StudioStart was not called before.
	 *
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INTERNAL if an exception occured
	 */
	StudioStatus StudioStop();

	// Show
	/**
	 * Returns the state of a given show
	 *
	 * @param   show_id id of the show to get
	 * @return       OK if successful
	 *               SHOW_NOT_FOUND if show_id is not found in the shows map
	 *               StudioStatus::INTERNAL if an exception occured or a scene is NULL
	 */
	StudioStatus GetShow(string show_id);

	/**
	 * Creates a new empty show and adds it to the shows map.
	 *
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INTERNAL if an exception occured
	 */
	StudioStatus ShowCreate(string show_name);

	/**
	 * Creates a new show from an existing one and adds it to the shows map. It
	 * will have the same name with a different id. All scenes and sources are
	 * also duplicated. The new show is not set as active and is not started.
	 *
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INTERNAL if an exception occured or the original show is not found
	 */
	StudioStatus ShowDuplicate(string show_id);

	/**
	 * Calls removeShow to remove a show.
	 *
	 * @param   req  ShowRemoveRequest containing the show_id to remove.
	 * @param   rep  Empty response gRPC type.
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INTERNAL if an exception occured or the show is not found
	 */
	StudioStatus ShowRemove(string show_id);

	/**
	 * Calls loadShow to load a show.
	 *
	 * @param   req  ShowLoadRequest containing the path to load the show from
	 *               (named show_id).
	 * @param   rep  the show state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INTERNAL if an exception occured or the show failed to load
	 */
	StudioStatus ShowLoad(string show_path);

	// Scene
	/**
	 * Returns the state of a given scene to the gRPC caller.
	 *
	 * @param   req  SceneGetRequest containing the show_id and scene_id.
	 * @param   rep  the scene state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND show_id is not found in the show
	 *               map or if scene_id is not found in show_id.
	 *               StudioStatus::INTERNAL if an exception occured
	 */
	StudioStatus GetScene(string show_id, string scene_id);

	/**
	 * Creates a new empty scene and adds it to the scenes map of a given show.
	 *
	 * @param   req  SceneAddRequest containing the show_id and new scene_name.
	 * @param   rep  the scene state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND show_id is not found in the show map
	 *               StudioStatus::INTERNAL if an exception occured or failed to
	 *               add scene
	 */
	StudioStatus SceneAdd(string show_id, string scene_name);

	/**
	 * Creates a new scene from an existing one and adds it to the scenes map of
	 * a given show. It will have the same name with a different id. All sources
	 * are also duplicated. The new scene is not set as active and is not started.
	 *
	 * @param   req  SceneDuplicateRequest containing the show_id where the
	 *               scene to duplicate is located, and its scene_id.
	 * @param   rep  the scene state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND if show_id is not found
	 *               StudioStatus::INTERNAL if an exception occured or the
	 *               original scene is not found
	 */
	StudioStatus SceneDuplicate(string show_id, string scene_id);

	/**
	 * Removes a given scene from a given show.
	 *
	 * @param   req  SceneRemoveRequest containing the show_id that contains the
	 *               scene_id to remove.
	 * @param   rep  Empty response gRPC type.
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND if show_id or scene_id is not found
	 *               StudioStatus::FAILED_PRECONDITION if the scene is active
	 *               StudioStatus::INTERNAL if an exception occured
	 */
	StudioStatus SceneRemove(string show_id, string scene_name);

	/**
	 * Sets a given scene as active in a given show : the show switches to this
	 * scene.
	 *
	 * @param   req  SceneSetAsCurrentRequest containing the show_id that
	 *               contains the sceene, and scene_id of the scene to switch to.
	 * @param   rep  the show state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND if show_id or scene_id is not found
	 *               StudioStatus::INVALID_ARGUMENT if the scene is already active
	 *               StudioStatus::INTERNAL if an exception occured or the scene transition failed
	 */
	StudioStatus SceneSetAsCurrent(string show_id, string scene_id);

	/**
	 * Returns the id of the currently active scene t othe gRPC caller.
	 *
	 * @param   req  SceneGetCurrentRequest containing the show_id
	 * @param   rep  SceneGetCurrentResponse containing the active scene_id.
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND if show_id is not found
	 *               StudioStatus::INTERNAL if an exception occured or the active scene is NULL
	 */
	StudioStatus SceneGetCurrent(string show_id);

	// Source
	/**
	 * Returns the state of a given source to the gRPC caller.
	 *
	 * @param   req  SourceGetRequest containing the show_id, scene_id and source_id
	 * @param   rep  the source state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND if show_id is not found in the show
	 *               map or if scene_id is not found in show_id or if source_id
	 *               is not found in scene_id
	 *               StudioStatus::INTERNAL if an exception occured
	 */
	StudioStatus GetSource(string show_id, string scene_id, string source_id);

	/**
	 * Creates a new empty source and adds it to the source map of a given scene
	 * in a given show.
	 *
	 * @param   req  SourceAddRequest containing the show_id and scene_id, with
	 *               the new source_name, source_type, and source_url.
	 * @param   rep  the source state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::INVALID_ARGUMENT if the source_type is not supported
	 *               StudioStatus::NOT_FOUND if show_id is not found in the show
	 *               map or if scene_id is not found in show_id
	 *               StudioStatus::INTERNAL if an exception occured or failed to
	 *               add source
	 */
	StudioStatus SourceAdd(string show_id, string scene_id, string source_name, string source_type, string source_url);

	/**
	 * Creates a new source from an existing one and adds it to the source map
	 * of a given scene in a given show. It will have the same name, type and
	 * url with a different id. The new source is not set as active not started.
	 *
	 * @param   req  SourceDuplicateRequest containing the show_id and scene_id
	 *               where the source to duplicate is located, and its source_id.
	 * @param   rep  the source state (see proto/studio.proto).
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND if show_id or scene_id is not found
	 *               StudioStatus::INTERNAL if an exception occured or the
	 *               original source is not found
	 */
	StudioStatus SourceDuplicate(string show_id, string scene_id, string source_id);

	/**
	 * Removes a given source from a given scene in a given show.
	 *
	 * @param   req  SourceRemoveRequest containing the show_id and scene_id
	 *               that contain the source_id to remove.
	 * @param   rep  Empty response gRPC type.
	 * @return       StudioStatus::OK if successful
	 *               StudioStatus::NOT_FOUND if show_id, scene_id or source_id is not found
	 *               StudioStatus::FAILED_PRECONDITION if the source is active
	 *               StudioStatus::INTERNAL if an exception occured
	 */
	StudioStatus SourceRemove(string show_id, string scene_id, string source_id);

    // TODO doc
	StudioStatus SourceSetProperties(string show_id, string scene_id, string source_id, string source_type, string source_url);

private:
	//Initializes obs: reset video and audio context, load modules libs, create RTMP output and encoders. Then starts the currently active show.
	StudioStatus studioInit();
	// Releases the obs output and encoders, stop the currently active show, stops obs.
	StudioStatus studioRelease();
	Show* getShow(string show_id);
	Show* addShow(string show_name);
	Show* loadShow(string show_id);
	Show* duplicateShow(string show_id);
	StudioStatus removeShow(string show_id);
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