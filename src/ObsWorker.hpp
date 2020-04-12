#pragma once

#include "Trace.hpp"
#include "Settings.hpp"

using namespace std;

class ObsWorker {
public:
    ObsWorker(Settings settings);
    ~ObsWorker();
    int start();

private:
    static void do_work(Settings settings); // worker thread
    static int LoadModule(const char* binPath, const char* dataPath);
    Settings settings;
};