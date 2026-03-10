#include "device_engine/device_engine.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "light/virtual_light.hpp"
#include "thermostat/virtual_thermostat.hpp"

using json = nlohmann::json;

DeviceEngine& DeviceEngine::instance() {
    static DeviceEngine inst;
    return inst;
}

int DeviceEngine::load_from_json(const std::string& models_path,
                                  const std::string& devices_path) {
    json models;
    {
        std::ifstream f(models_path);
        if (!f.is_open())
            throw std::runtime_error("[DeviceEngine] Cannot open: " + models_path);
        f >> models;
    }

    json devices;
    {
        std::ifstream f(devices_path);
        if (!f.is_open())
            throw std::runtime_error("[DeviceEngine] Cannot open: " + devices_path);
        f >> devices;
    }

    int count = 0;
    for (const auto& d : devices) {
        const std::string id    = d.at("id");
        const std::string label = d.at("label");
        const std::string room  = d.at("room");
        const std::string model = d.at("model");

        if (!models.contains(model)) {
            std::cerr << "[DeviceEngine] Unknown model '" << model << "' — skipping " << id << "\n";
            continue;
        }

        const auto& m            = models.at(model);
        const std::string type   = m.at("type");
        std::string       protocol = m.at("protocol");

        // Trim trailing whitespace (device_models.json has trailing spaces on some values).
        while (!protocol.empty() && std::isspace(static_cast<unsigned char>(protocol.back())))
            protocol.pop_back();

        std::vector<std::string> caps = m.at("capabilities").get<std::vector<std::string>>();

        std::unique_ptr<VirtualDevice> device;

        if (type == "light") {
            auto light = std::make_unique<VirtualLight>(id, label, room, model, protocol, caps);
            light->init_states();
            device = std::move(light);
        } else if (type == "thermostat") {
            auto thermo = std::make_unique<VirtualThermostat>(id, label, room, model, protocol, caps);
            thermo->init_states();
            device = std::move(thermo);
        } else {
            std::cerr << "[DeviceEngine] Unsupported type '" << type << "' — skipping " << id << "\n";
            continue;
        }

        std::cout << "[DeviceEngine] Loaded " << type << ": " << id << " (" << label << ", " << room << ")\n";
        devices_.emplace(id, std::move(device));
        ++count;
    }

    return count;
}

VirtualDevice* DeviceEngine::get_device(const std::string& device_id) {
    auto it = devices_.find(device_id);
    return it != devices_.end() ? it->second.get() : nullptr;
}

const std::unordered_map<std::string, std::unique_ptr<VirtualDevice>>&
DeviceEngine::devices() const {
    return devices_;
}

void DeviceEngine::update_device_state(const std::string& device_id, const Event& event) {
    VirtualDevice* device = get_device(device_id);
    if (!device) {
        std::cerr << "[DeviceEngine] Device not found: " << device_id << "\n";
        return;
    }
    device->update_state(event);
}
