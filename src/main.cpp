#include <QApplication>
#include <QPushButton>
#include "Studio.hpp"
#include "Trace.hpp"
#include "Settings.hpp"

using namespace std;

// TODO Will be overriden by settings
int gTraceLevel = TRACE_LEVEL_TRACE;
int gTraceFormat = TRACE_FORMAT_TEXT;

void switch_source(Studio& studio) {
    StudioStatus s;

    trace_info("Switching source...");
    Show* show = studio.GetActiveShow();
    if(!show) {
        trace_warn("No active show");
        return;
    }

    SceneMap scenes = show->Scenes();
    Scene* current_scene = studio.SceneGetCurrent(show->Id());

    if(!current_scene) {
        trace_error("Failed to get current scene", field_s(show->Id()));
    } else {
        Scene* next_scene = nullptr;
        for(auto& [scene_id, scene] : scenes) {
            if(scene_id != current_scene->Id()) {
                next_scene = scene;
                break;
            }
        }
        s = studio.SceneSetAsCurrent(show->Id(), next_scene->Id());
        if(!s.ok()) {
            trace_error("Failed to switch to scene", field_s(next_scene->Id()));
        } else {
            trace_info("Switched to scene", field_s(next_scene->Id()));
        }
    }
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    try {
        Settings settings = LoadConfig(OBS_HEADLESS_PATH "config.txt");

        Studio studio(&settings);
        StudioStatus s;
        string show_path = OBS_HEADLESS_PATH "shows/default.json";

        trace_info("Load show", field_s(show_path));

        Show* show = studio.ShowLoad(show_path);
        if(!show) {
            throw runtime_error("Failed to load show");
        }

        trace_info("Loaded show", field_s(show->Id()), field_s(show->Name()));

        trace("Start Studio");
        s = studio.StudioStart();
        if(!s.ok()) {
            throw runtime_error("Failed to start studio");
        }
        
        char c = 0;
        // Wait for 'q' to stop the thread
        do {
            switch(c) {
                case 's':
                    switch_source(studio);
                    break;

                case 'q':
                    trace_info("Exiting");
                    break;

                default:
                    trace_info("Press 'q' key to stop");
            }
            
            c = cin.get();
        } while (c != 'q');

        s = studio.StudioStop();
        if(!s.ok()) {
            throw runtime_error("Failed to stop studio");
        }
    }
    catch(const exception& e) {
        trace_error("An exception occured: ", field_ns("exception", e.what()));
    }
    catch(const string& e) {
        trace_error("An exception occured: ", field_ns("exception", e.c_str()));
    }
    catch(...) {
        trace_error("An uncaught exception occured !");
    }


    trace("Exit app");
    return 0;
}
