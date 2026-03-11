#pragma once

#include <string>
#include <thread>

// Forward-declare to avoid including httplib.h in the header.
namespace httplib { class Server; }

// SimulatorApi — Singleton.
// Exposes an internal HTTP REST API on :4000 (CORE_PORT).
// Runs the httplib server on a dedicated background thread so the
// main event-scheduler loop is never blocked.
//
// Lifecycle:
//   SimulatorApi::instance().start(port);   // non-blocking
//   SimulatorApi::instance().stop();        // called from signal handler
class SimulatorApi {
public:
    static SimulatorApi& instance();

    SimulatorApi(const SimulatorApi&)            = delete;
    SimulatorApi& operator=(const SimulatorApi&) = delete;
    SimulatorApi(SimulatorApi&&)                 = delete;
    SimulatorApi& operator=(SimulatorApi&&)      = delete;

    // Registers all routes and starts listening on a background thread.
    // No-op if already started.
    void start(int port);

    // Stops the HTTP server and joins the background thread.
    // Safe to call from a signal handler via std::atomic stop flag.
    void stop();

private:
    SimulatorApi();
    ~SimulatorApi();

    void register_routes();

    std::unique_ptr<httplib::Server> svr_;
    std::thread                      thread_;
};
