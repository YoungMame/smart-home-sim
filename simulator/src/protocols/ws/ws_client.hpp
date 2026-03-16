#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "core/adapter_manager/protocol_client.hpp"

class WsClient : public ProtocolClient {
public:
    explicit WsClient(std::string device_id = "");
    ~WsClient() override;

    AdapterProtocol protocol() const override;

    void connect(const std::string& endpoint) override;
    void disconnect() override;
    bool is_connected() const override;

    void send(const std::string& channel, const std::string& payload) override;

    std::vector<SimulatedMessage> messages() const override;
    void clear_messages() override;

private:
    mutable std::mutex            mutex_;
    std::string                   endpoint_;
    std::vector<SimulatedMessage> messages_;
};
