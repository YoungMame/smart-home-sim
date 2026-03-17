#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class IpManager {
public:
    explicit IpManager(std::uint8_t first_ip_byte = 2);

    ~IpManager() = default;

    IpManager(const IpManager&) = delete;
    IpManager& operator=(const IpManager&) = delete;

    std::uint8_t assign_ip_byte(const std::string& device_id);

    std::optional<std::uint8_t> ip_byte_for(const std::string& device_id) const;

    bool has_device(const std::string& device_id) const;
    void remove_device(const std::string& device_id);
    void clear();

private:
    static std::vector<std::uint8_t> build_pool(std::uint8_t first_ip_byte);

    mutable std::mutex                               mutex_;
    std::vector<std::uint8_t>                        ip_pool_;
    std::set<std::uint8_t>                           available_ip_bytes_;
    std::unordered_map<std::string, std::uint8_t>    ip_byte_by_device_id_;
};