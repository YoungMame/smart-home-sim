#include "device_engine/device_engine.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

#include "core/adapter_manager/adapter_manager.hpp"
#include "db/sqlite_store.hpp"
#include "light/virtual_light.hpp"
#include "motion_sensor/virtual_motion_sensor.hpp"
#include "thermostat/virtual_thermostat.hpp"

std::shared_ptr<VirtualDevice> DeviceEngine::create_device(const std::string& id,
                                                           const std::string& label,
                                                           const std::string& room,
                                                           const VirtualDeviceModel& model) const {
    std::shared_ptr<VirtualDevice> device;

    switch (device_type_from_string(model.type)) {
        case DeviceType::Light: {
            auto light = std::make_shared<VirtualLight>(id, label, room, &model);
            light->init_states();
            device = std::move(light);
            break;
        }
        case DeviceType::Thermostat: {
            auto thermostat = std::make_shared<VirtualThermostat>(id, label, room, &model);
            thermostat->init_states();
            device = std::move(thermostat);
            break;
        }
        case DeviceType::Motion: {
            auto motion_sensor = std::make_shared<VirtualMotionSensor>(id, label, room, &model);
            motion_sensor->init_states();
            device = std::move(motion_sensor);
            break;
        }
        case DeviceType::Unknown:
            return nullptr;
    }

    return device;
}

void DeviceEngine::log_devices() const {
    const auto devices = snapshot_devices();
    for (const auto& device_ptr : devices) {
        const VirtualDevice* device = device_ptr.get();
        std::cout << "[DeviceEngine] " << device->type() << ": " << device->id() << " (" << device->label() << ", " << device->room() << ")\n";
    }
}

DeviceType device_type_from_string(const std::string& type) {
    if (type == "light")      return DeviceType::Light;
    if (type == "thermostat") return DeviceType::Thermostat;
    if (type == "motion")     return DeviceType::Motion;
    return DeviceType::Unknown;
}

DeviceEngine& DeviceEngine::instance() {
    static DeviceEngine inst;
    return inst;
}

int DeviceEngine::load_from_db(const std::string& db_path, const std::string& seed_path) {
    SqliteStore::initialize(db_path, seed_path);

    const auto models = SqliteStore::load_models(db_path);
    const auto device_rows = SqliteStore::load_devices(db_path);

    int count = 0;
    std::lock_guard<std::mutex> lock(mutex_);
    db_path_ = db_path;

    for (const auto& [existing_id, _] : devices_) {
        AdapterManager::instance().unregister_device(existing_id);
    }

    models_.clear();
    devices_.clear();

    for (const auto& model : models) {
        models_.emplace(model.id, model);
    }

    for (const auto& row : device_rows) {
        const std::string& id = row.id;
        const std::string& label = row.label;
        const std::string& room = row.room;
        const std::string& model_id = row.model_id;

        if (models_.find(model_id) == models_.end()) {
            std::cerr << "[DeviceEngine] Unknown model '" << model_id << "' — skipping " << id << "\n";
            continue;
        }

        const VirtualDeviceModel& vm = models_.at(model_id);
        std::shared_ptr<VirtualDevice> device = create_device(id, label, room, vm);
        if (!device) {
            std::cerr << "[DeviceEngine] Unsupported type '" << vm.type << "' — skipping " << id << "\n";
            continue;
        }

        std::cout << "[DeviceEngine] Loaded " << device->type() << ": " << device->id() << " (" << label << ", " << room << ")\n";
        devices_.emplace(id, std::move(device));
        ++count;
    }

    return count;
}

VirtualDevice* DeviceEngine::get_device(const std::string& device_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(device_id);
    return it != devices_.end() ? it->second.get() : nullptr;
}

std::shared_ptr<VirtualDevice> DeviceEngine::get_device_shared(const std::string& device_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = devices_.find(device_id);
    return it != devices_.end() ? it->second : nullptr;
}

bool DeviceEngine::has_device(const std::string& device_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return devices_.find(device_id) != devices_.end();
}

bool DeviceEngine::add_device(const std::string& id,
                              const std::string& label,
                              const std::string& room,
                              const std::string& model_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (db_path_.empty()) {
        std::cerr << "[DeviceEngine] DB is not initialized\n";
        return false;
    }

    if (devices_.find(id) != devices_.end()) {
        std::cerr << "[DeviceEngine] Device already exists: " << id << "\n";
        return false;
    }

    auto model_it = models_.find(model_id);
    if (model_it == models_.end()) {
        std::cerr << "[DeviceEngine] Unknown model: " << model_id << "\n";
        return false;
    }

    std::shared_ptr<VirtualDevice> device = create_device(id, label, room, model_it->second);
    if (!device) {
        std::cerr << "[DeviceEngine] Unsupported type '" << model_it->second.type << "'\n";
        return false;
    }

    if (!SqliteStore::insert_device(db_path_, id, label, room, model_id)) {
        std::cerr << "[DeviceEngine] Failed to persist device in DB: " << id << "\n";
        return false;
    }

    devices_.emplace(id, std::move(device));
    return true;
}

bool DeviceEngine::remove_device(const std::string& device_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (db_path_.empty()) {
        std::cerr << "[DeviceEngine] DB is not initialized\n";
        return false;
    }

    if (!SqliteStore::delete_device(db_path_, device_id)) {
        return false;
    }

    AdapterManager::instance().unregister_device(device_id);
    return devices_.erase(device_id) > 0;
}

const std::unordered_map<std::string, std::shared_ptr<VirtualDevice>>&
DeviceEngine::devices() const {
    return devices_;
}

std::vector<std::shared_ptr<VirtualDevice>> DeviceEngine::snapshot_devices() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<VirtualDevice>> snapshot;
    snapshot.reserve(devices_.size());
    for (const auto& [id, device] : devices_)
        snapshot.push_back(device);
    return snapshot;
}

std::vector<VirtualDeviceModel> DeviceEngine::snapshot_models() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<VirtualDeviceModel> snapshot;
    snapshot.reserve(models_.size());
    for (const auto& [id, model] : models_)
        snapshot.push_back(model);
    return snapshot;
}

void DeviceEngine::update_device_state(const std::string& device_id, const Event& event) {
    std::shared_ptr<VirtualDevice> device = get_device_shared(device_id);
    if (!device) {
        std::cerr << "[DeviceEngine] Device not found: " << device_id << "\n";
        return;
    }
    device->update_state(event);
}
