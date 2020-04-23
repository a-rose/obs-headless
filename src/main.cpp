#include <QApplication>
#include <QPushButton>
#include "Studio.hpp"
#include "Trace.hpp"
#include "Settings.hpp"

using namespace std;

// TODO Will be overriden by settings
int gTraceLevel = TRACE_LEVEL_TRACE;
int gTraceFormat = TRACE_FORMAT_TEXT;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    try {
        Settings settings = LoadConfig(OBS_HEADLESS_PATH "config.txt");

        Studio studio(&settings);
        StudioStatus s;
        string show_path = OBS_HEADLESS_PATH "shows/default.json";

        trace("Load show", field_s(show_path));
        s = studio.ShowLoad(show_path);
        if(!s.ok()) {
            throw runtime_error("Failed to load show");
        }

        trace("Start Studio");
        s = studio.StudioStart();
        if(!s.ok()) {
            throw runtime_error("Failed to start studio");
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
