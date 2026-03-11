#include "simulator_api.hpp"
#include <iostream>
#include "httplib.h"
#include <nlohmann/json.hpp>
#include "core/device_engine/device_engine.hpp"

using json = nlohmann::json;

using namespace httplib;

SimulatorApi& SimulatorApi::instance() {
    static SimulatorApi inst;
    return inst;
}

SimulatorApi::SimulatorApi(): svr_(std::make_unique<Server>()) {}

SimulatorApi::~SimulatorApi() {
    stop();
}

void SimulatorApi::start(int port) {
    if (thread_.joinable()) return; // already running

    register_routes();

    thread_ = std::thread([this, port]() {
        std::cout << "[SimulatorApi] Listening on :" << port << "\n";
        svr_->listen("0.0.0.0", port);
        std::cout << "[SimulatorApi] Server stopped.\n";
    });
}

void SimulatorApi::stop() {
    if (svr_) svr_->stop();
    if (thread_.joinable()) thread_.join();
}

// Routes
void SimulatorApi::register_routes() {

    svr_->Get("/health", [](const Request&, Response& res) {
        res.set_content(json{{"status", "ok"}}.dump(), "application/json");
    });

    svr_->Get("/devices", [](const Request&, Response& res) {
        const auto& devices = DeviceEngine::instance().devices();

        json body = json::array();
        for (const auto& [id, device] : devices) {
            body.push_back({
                {"id",      std::string(device->id())},
                {"label",    std::string(device->label())},
                {"room",     std::string(device->room())},
                {"model",    std::string(device->model()->label)},
                {"protocol", std::string(device->protocol())}
            });
        }

        res.set_content(body.dump(), "application/json");
    });

    svr_->Get("/devices/:id", [](const Request& req, Response& res) {
        auto id_it = req.path_params.find("id");
        const std::string id = id_it != req.path_params.end() ? id_it->second : "";
        VirtualDevice* device = DeviceEngine::instance().get_device(id);

        if (!device) {
            res.status = 404;
            res.set_content("{\"error\":\"device not found\"}", "application/json");
            return;
        }

        json body = {
            {"id",       std::string(device->id())},
            {"label",    std::string(device->label())},
            {"room",     std::string(device->room())},
            {"model",    std::string(device->model()->label)},
            {"protocol", std::string(device->protocol())}
        };

        res.set_content(body.dump(), "application/json");
    });
}
