#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "core/scheduler/event_scheduler.hpp"
#include "core/device_engine/device_engine.hpp"
#include "server/simulator_api.hpp"
#include "cli/cli.hpp"

static void signal_handler(int /*sig*/) {
    SimulatorApi::instance().stop();
    EventScheduler::instance().stop();
}

namespace {
} // namespace

int main() {
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "[main] Smart Home Simulator starting...\n";

    // Data directory: use SMART_HOME_DATA_DIR env var if set, otherwise fall back to "data".
    const char* env_data_dir = std::getenv("SMART_HOME_DATA_DIR");
    const std::string data_dir = env_data_dir ? env_data_dir : "data";

    const char* env_db_path = std::getenv("SMART_HOME_DB_PATH");
    const std::string db_path = env_db_path ? env_db_path : (data_dir + "/simulator.db");

    const char* env_seed_path = std::getenv("SMART_HOME_DB_SEED");
    const std::string seed_path = env_seed_path ? env_seed_path : (data_dir + "/seed.sql");

    const int loaded = DeviceEngine::instance().load_from_db(db_path, seed_path);
    std::cout << "[main] " << loaded << " virtual device(s) loaded.\n";

    // TODO: initialize AdapterManager (MQTT and REST adapters)

    const char* env_port = std::getenv("CORE_PORT");
    const int api_port = env_port ? std::stoi(env_port) : 4000;
    SimulatorApi::instance().start(api_port);

    std::thread scheduler_thread([] {
        EventScheduler::instance().run();
    });

    CLI::run_interactive_loop();

    SimulatorApi::instance().stop();
    EventScheduler::instance().stop();
    if (scheduler_thread.joinable()) {
        scheduler_thread.join();
    }

    std::cout << "[main] Simulator shut down cleanly.\n";
    return 0;
}
