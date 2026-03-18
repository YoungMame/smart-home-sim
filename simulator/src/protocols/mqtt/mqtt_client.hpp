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
    static void on_connect_static(struct mosquitto* mosq, void* obj, int rc);
    static void on_message_static(struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg);

    void on_connect(int rc);
    void on_message(const struct mosquitto_message* msg);

    mutable std::mutex            mutex_;
    std::vector<SimulatedMessage> messages_;
    mosquitto*                    mosq_{nullptr};
    bool                          loop_started_{false};
};