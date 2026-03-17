#pragma once

#include <string>
#include <vector>

#include "virtual_device_model.hpp"

struct DeviceRow {
    std::string id;
    std::string label;
    std::string room;
    std::string model_id;
};

class SqliteStore {
public:
    SqliteStore() = delete;
    ~SqliteStore() = delete;
    SqliteStore(const SqliteStore&) = delete;
    SqliteStore& operator=(const SqliteStore&) = delete;

    static void initialize(const std::string& db_path, const std::string& seed_path);

    static std::vector<VirtualDeviceModel> load_models(const std::string& db_path);
    static std::vector<DeviceRow> load_devices(const std::string& db_path);

    static bool insert_device(const std::string& db_path,
                              const std::string& id,
                              const std::string& label,
                              const std::string& room,
                              const std::string& model_id);

    static bool delete_device(const std::string& db_path, const std::string& id);
};
