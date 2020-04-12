#include <QApplication>
#include <QPushButton>
#include "ObsWorker.hpp"
#include "Trace.hpp"
#include "Settings.hpp"

using namespace std;


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    try {
        Settings settings = LoadConfig(OBS_HEADLESS_PATH "config.txt");
        trace("Start ObsWorker");
        ObsWorker worker(settings);
        worker.start();
    }
    catch(const exception& e) {
        trace_error("An exception occured: %s", e.what());
    }
    catch(const string& e) {
        trace_error("An exception occured: %s", e.c_str());
    }
    catch(...) {
        trace_error("An uncaught exception occured !");
    }


    trace("Exit app");
    return 0;
}
