#include <time.h>
#include <stdio.h>
#include <wchar.h>
#include <signal.h>
#include <unistd.h>
#include <chrono>
#include <ratio>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>
#include <thread>         // std::thread
#include <condition_variable>
#include <vector>

#include "obs.h"
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