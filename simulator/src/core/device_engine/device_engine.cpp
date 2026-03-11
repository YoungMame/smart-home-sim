#include "device_engine/device_engine.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "light/virtual_light.hpp"
#include "thermostat/virtual_thermostat.hpp"

using json = nlohmann::json;

void DeviceEngine::log_devices() const {
    for (const auto& pair : devices_) {
        const VirtualDevice* device = pair.second.get();
        std::cout << "[DeviceEngine] " << device->type() << ": " << device->id() << " (" << device->label() << ", " << device->room() << ")\n";
    }
}

DeviceType device_type_from_string(const std::string& type) {
    if (type == "light")      return DeviceType::Light;
    if (type == "thermostat") return DeviceType::Thermostat;
    return DeviceType::Unknown;
}

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

    // Models must be loaded before devices since devices reference them by id.
    for (auto& [model_id, m] : models.items()) {
        std::string protocol = m.at("protocol");
        while (!protocol.empty() && std::isspace(static_cast<unsigned char>(protocol.back())))
            protocol.pop_back();

        VirtualDeviceModel vm;
        vm.id           = model_id;
        vm.label        = m.value("label", model_id);
        vm.type         = m.at("type");
        vm.protocol     = std::move(protocol);
        vm.capabilities = m.at("capabilities").get<std::vector<std::string>>();
        models_.emplace(model_id, std::move(vm));
    }

    for (const auto& d : devices) {
        const std::string id       = d.at("id");
        const std::string label    = d.at("label");
        const std::string room     = d.at("room");
        const std::string model_id = d.at("model");

        if (models_.find(model_id) == models_.end()) {
            std::cerr << "[DeviceEngine] Unknown model '" << model_id << "' — skipping " << id << "\n";
            continue;
        }

        const VirtualDeviceModel* vm = &models_.at(model_id);

        std::unique_ptr<VirtualDevice> device;

        switch (device_type_from_string(vm->type)) {
            case DeviceType::Light: {
                auto d = std::make_unique<VirtualLight>(id, label, room, vm);
                d->init_states();
                device = std::move(d);
                break;
            }
            case DeviceType::Thermostat: {
                auto d = std::make_unique<VirtualThermostat>(id, label, room, vm);
                d->init_states();
                device = std::move(d);
                break;
            }
            case DeviceType::Unknown:
                std::cerr << "[DeviceEngine] Unsupported type '" << vm->type << "' — skipping " << id << "\n";
                continue;
        }

        std::cout << "[DeviceEngine] Loaded " << device->type() << ": " << device->id() << " (" << label << ", " << room << ")\n";
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
