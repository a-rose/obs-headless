#include <csignal>
#include <QApplication>
#include <QPushButton>
#include "lib/Studio.hpp"
#include "lib/Trace.hpp"
#include "lib/Settings.hpp"

using namespace std;

unique_ptr<Server> server = nullptr;

// TODO Will be overriden by settings
int gTraceLevel = TRACE_LEVEL_TRACE;
int gTraceFormat = TRACE_FORMAT_TEXT;

void intHandler(int dummy) {
	if(server != nullptr) {
		trace_info("Stopping the server");
		server->Shutdown();
	}
}

void RunServer(Settings* settings) {
	string server_address("0.0.0.0:50051"); // TODO
	Studio service(settings);

	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	// Finally assemble the server.
	server = builder.BuildAndStart();
	trace_info("gRPC Server listening", field_s(server_address));

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	server->Wait();
}

int main(int argc, char *argv[]) {
	bool ret;
	char* end;

	QApplication app(argc, argv);
	signal(SIGINT, intHandler);

	try {
        Settings settings = LoadConfig(OBS_HEADLESS_PATH "/etc/config.txt");
		RunServer(&settings);
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

    trace("Exit server");
    return 0;
}
