#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>

#include "core/scheduler/event_scheduler.hpp"
#include "core/device_engine/device_engine.hpp"

// Forward the OS signal to the scheduler so run() exits cleanly.
static void signal_handler(int /*sig*/) {
    EventScheduler::instance().stop();
}

int main() {
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "[main] Smart Home Simulator starting...\n";

    // Data directory: use SMART_HOME_DATA_DIR env var if set, otherwise fall back to "data".
    const char* env_data_dir = std::getenv("SMART_HOME_DATA_DIR");
    const std::string data_dir = env_data_dir ? env_data_dir : "data";

    const int loaded = DeviceEngine::instance().load_from_json(
        data_dir + "/device_models.json",
        data_dir + "/devices.json"
    );
    std::cout << "[main] " << loaded << " virtual device(s) loaded.\n";

    // TODO: initialize AdapterManager (MQTT, REST, WS adapters)
    // TODO: start Simulator API HTTP server on port CORE_PORT (logs, data, event triggers)

    std::cout << "[main] Event scheduler running. Press Ctrl+C to stop.\n";

    EventScheduler::instance().run(); // blocking — returns when stop() is called

    std::cout << "[main] Simulator shut down cleanly.\n";
    return 0;
}
