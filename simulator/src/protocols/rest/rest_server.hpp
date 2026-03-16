#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "core/adapter_manager/protocol_client.hpp"
#include "rest_client.hpp"

class RestServer : public ProtocolClient {
public:
    RestServer(std::string device_id);
    ~RestServer() override;

    AdapterProtocol protocol() const override;

    void connect(const std::string& endpoint) override;
    void disconnect() override;
    bool is_connected() const override;

    void send(const std::string& route, const std::string& payload) override;

    std::vector<SimulatedMessage> messages() const override;
    void clear_messages() override;

private:
    mutable std::mutex            mutex_;
    std::string                   endpoint_;
    RestClient                    client_;
    std::vector<SimulatedMessage> messages_;
};
