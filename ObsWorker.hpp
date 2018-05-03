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
#include "trace.h"

using namespace std;



typedef struct worker_settings {
    string stream_a;
    string stream_b;
    string server;
    string key;
    int transition_delay_sec;
    int transition_duration_ms;
} worker_settings_t;

class ObsWorker {
public:
    ObsWorker(worker_settings_t settings);
    ~ObsWorker();
    int start();

private:
    static void do_work(worker_settings_t settings); // worker thread
    worker_settings_t settings;
};