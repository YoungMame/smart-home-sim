#include "ip_manager.hpp"

#include <stdexcept>

IpManager::IpManager(std::uint8_t first_ip_byte)
    : ip_pool_(build_pool(first_ip_byte)),
      available_ip_bytes_(ip_pool_.begin(), ip_pool_.end()) {
}

// Used to assign a local IP byte to a device. If the device already has an assigned IP byte, it returns the existing one.
std::uint8_t IpManager::assign_ip_byte(const std::string& device_id) {
    if (device_id.empty()) {
        throw std::invalid_argument("Device id cannot be empty");
    }

    std::scoped_lock lock(mutex_);
    const auto existing = ip_byte_by_device_id_.find(device_id);
    if (existing != ip_byte_by_device_id_.end()) {
        return existing->second;
    }

    if (available_ip_bytes_.empty()) {
        throw std::runtime_error("No available local IP bytes left for assignment");
    }

    const std::uint8_t ip_byte = *available_ip_bytes_.begin();
    available_ip_bytes_.erase(available_ip_bytes_.begin());
    ip_byte_by_device_id_.emplace(device_id, ip_byte);
    return ip_byte;
}

// Returns the IP byte assigned to the device, or std::nullopt if the device is unknown.
std::optional<std::uint8_t> IpManager::ip_byte_for(const std::string& device_id) const {
    std::scoped_lock lock(mutex_);
    const auto it = ip_byte_by_device_id_.find(device_id);
    if (it == ip_byte_by_device_id_.end()) {
        return std::nullopt;
    }

    return it->second;
}

// Checks if the device has an assigned IP byte in the manager.
bool IpManager::has_device(const std::string& device_id) const {
    std::scoped_lock lock(mutex_);
    return ip_byte_by_device_id_.find(device_id) != ip_byte_by_device_id_.end();
}

// Removes the device and its assigned IP byte from the manager, making the IP byte available for future assignments.
void IpManager::remove_device(const std::string& device_id) {
    std::scoped_lock lock(mutex_);
    const auto it = ip_byte_by_device_id_.find(device_id);
    if (it == ip_byte_by_device_id_.end()) {
        return;
    }

    available_ip_bytes_.insert(it->second);
    ip_byte_by_device_id_.erase(it);
}

// Clears all assigned IP bytes and resets the cursor for the next assignment.
void IpManager::clear() {
    std::scoped_lock lock(mutex_);
    ip_byte_by_device_id_.clear();
    available_ip_bytes_.clear();
    available_ip_bytes_.insert(ip_pool_.begin(), ip_pool_.end());
}

// A pool of local IP bytes is built starting from the specified first byte up to simulated network IP mask.
std::vector<std::uint8_t> IpManager::build_pool(std::uint8_t first_ip_byte) {
    if (first_ip_byte < 2 || first_ip_byte > 254) {
        throw std::invalid_argument("First local IP byte must be in range 2..254");
    }

    std::vector<std::uint8_t> pool;
    pool.reserve(static_cast<std::size_t>(255 - first_ip_byte));
    for (std::uint16_t value = first_ip_byte; value <= 254; ++value) {
        pool.push_back(static_cast<std::uint8_t>(value));
    }

    return pool;
}