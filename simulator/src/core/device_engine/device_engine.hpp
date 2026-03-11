#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "virtual_device.hpp"
#include "event_engine/event.hpp"

enum class DeviceType {
    Light,
    Thermostat,
    Unknown,
};

DeviceType device_type_from_string(const std::string& type);

// DeviceEngine — Singleton.
// Owns all VirtualDevice instances and dispatches events to them.
class DeviceEngine {
public:
    static DeviceEngine& instance();

    DeviceEngine(const DeviceEngine&)            = delete;
    DeviceEngine& operator=(const DeviceEngine&) = delete;
    DeviceEngine(DeviceEngine&&)                 = delete;
    DeviceEngine& operator=(DeviceEngine&&)      = delete;

    // Parse device_models.json then devices.json and instantiate
    // the appropriate VirtualDevice subclass for each entry.
    // Returns the number of devices successfully loaded.
    int load_from_json(const std::string& models_path, const std::string& devices_path);

    // Returns nullptr if device_id is unknown.
    VirtualDevice* get_device(const std::string& device_id);

    const std::unordered_map<std::string, std::unique_ptr<VirtualDevice>>& devices() const;

    // Forward an event to the target device.
    void update_device_state(const std::string& device_id, const Event& event);

private:
    DeviceEngine() = default;

    std::unordered_map<std::string, std::unique_ptr<VirtualDevice>> devices_;
};
