#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

class WsServersManager {
public:
    explicit WsServersManager(int start_port = 51000);

    std::string assign_endpoint(const std::string& device_id,
                                const std::string& base_endpoint);

    std::optional<std::string> endpoint_for(const std::string& device_id) const;

    bool has_device(const std::string& device_id) const;
    void remove_device(const std::string& device_id);
    void clear();

private:
    static std::string normalize_base(const std::string& base_endpoint);

    mutable std::mutex                           mutex_;
    const int                                    start_port_;
    int                                          next_port_;
    std::unordered_map<std::string, std::string> endpoints_by_device_id_;
};
