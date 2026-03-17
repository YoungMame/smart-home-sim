#pragma once

#include <mutex>
#include <string>
#include <vector>

#include <mosquitto.h>

#include "core/adapter_manager/protocol_client.hpp"

// Thin wrapper around libmosquitto.
class MQTTClient : public ProtocolClient {
public:
    MQTTClient();
    ~MQTTClient() override;

    MQTTClient(const MQTTClient&) = delete;
    MQTTClient& operator=(const MQTTClient&) = delete;

    AdapterProtocol protocol() const override;

    void connect(const std::string& endpoint) override;
    void disconnect() override;
    bool is_connected() const override;

    void send(const std::string& topic, const std::string& payload) override;

    std::vector<SimulatedMessage> messages() const override;
    void clear_messages() override;

    void connect(const std::string& host, int port);
    void publish(const std::string& topic, const std::string& message);

private:
    mutable std::mutex            mutex_;
    std::vector<SimulatedMessage> messages_;
    mosquitto* mosq_;
};