#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "core/adapter_manager/protocol_client.hpp"

class RestClient {
public:
    void connect(const std::string& endpoint);
    void disconnect();
    bool is_connected() const;

    SimulatedMessage get(const std::string& route, const std::string& payload = "");

    std::vector<SimulatedMessage> requests() const;
    void clear_requests();

private:
    mutable std::mutex            mutex_;
    std::string                   endpoint_;
    bool                          connected_{false};
    std::vector<SimulatedMessage> requests_;
};
