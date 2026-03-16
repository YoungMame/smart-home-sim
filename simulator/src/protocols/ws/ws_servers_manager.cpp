#include "ws_servers_manager.hpp"

#include <stdexcept>

WsServersManager::WsServersManager(int start_port)
    : start_port_(start_port), next_port_(start_port_) {
    if (start_port <= 0 || start_port > 65535) {
        throw std::invalid_argument("WS start port must be in range 1..65535");
    }
}

std::string WsServersManager::assign_endpoint(const std::string& device_id,
                                              const std::string& base_endpoint) {
    if (device_id.empty()) {
        throw std::invalid_argument("Device id cannot be empty");
    }

    std::scoped_lock lock(mutex_);
    const auto existing = endpoints_by_device_id_.find(device_id);
    if (existing != endpoints_by_device_id_.end()) {
        return existing->second;
    }

    if (next_port_ > 65535) {
        throw std::runtime_error("No available WS ports left for assignment");
    }

    const std::string normalized_base = normalize_base(base_endpoint);
    const std::string endpoint = normalized_base + ":" + std::to_string(next_port_) +
                                 "/devices/" + device_id;

    endpoints_by_device_id_.emplace(device_id, endpoint);
    ++next_port_;
    return endpoint;
}

std::optional<std::string> WsServersManager::endpoint_for(const std::string& device_id) const {
    std::scoped_lock lock(mutex_);
    const auto it = endpoints_by_device_id_.find(device_id);
    if (it == endpoints_by_device_id_.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool WsServersManager::has_device(const std::string& device_id) const {
    std::scoped_lock lock(mutex_);
    return endpoints_by_device_id_.find(device_id) != endpoints_by_device_id_.end();
}

void WsServersManager::remove_device(const std::string& device_id) {
    std::scoped_lock lock(mutex_);
    endpoints_by_device_id_.erase(device_id);
}

void WsServersManager::clear() {
    std::scoped_lock lock(mutex_);
    endpoints_by_device_id_.clear();
    next_port_ = start_port_;
}

std::string WsServersManager::normalize_base(const std::string& base_endpoint) {
    if (base_endpoint.empty()) {
        throw std::invalid_argument("Base endpoint cannot be empty");
    }

    std::string normalized = base_endpoint;
    while (!normalized.empty() && normalized.back() == '/') {
        normalized.pop_back();
    }

    if (normalized.empty()) {
        throw std::invalid_argument("Base endpoint cannot contain only '/'");
    }

    return normalized;
}
