#pragma once

#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "virtual_device.hpp"
#include "virtual_device_model.hpp"
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

    // Initializes DB (schema + seed) then loads models/devices from SQLite.
    // Returns the number of devices successfully loaded.
    int load_from_db(const std::string& db_path, const std::string& seed_path);

    // Returns nullptr if device_id is unknown.
    VirtualDevice* get_device(const std::string& device_id);

    std::shared_ptr<VirtualDevice> get_device_shared(const std::string& device_id);

    bool has_device(const std::string& device_id) const;

    bool add_device(const std::string& id,
                    const std::string& label,
                    const std::string& room,
                    const std::string& model_id);

    bool remove_device(const std::string& device_id);

    const std::unordered_map<std::string, std::shared_ptr<VirtualDevice>>& devices() const;
    std::vector<std::shared_ptr<VirtualDevice>> snapshot_devices() const;
    std::vector<VirtualDeviceModel> snapshot_models() const;

    // Forward an event to the target device.
    void update_device_state(const std::string& device_id, const Event& event);

    void log_devices() const;
private:
    DeviceEngine() = default;

    std::shared_ptr<VirtualDevice> create_device(const std::string& id,
                                                 const std::string& label,
                                                 const std::string& room,
                                                 const VirtualDeviceModel& model) const;

    mutable std::mutex mutex_;
    std::string db_path_;
    std::unordered_map<std::string, VirtualDeviceModel>             models_;
    std::unordered_map<std::string, std::shared_ptr<VirtualDevice>> devices_;
};
