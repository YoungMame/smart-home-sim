#pragma once

#include <string>
#include <mosquitto.h>

// Pure interface — implemented by MQTTClient (real) and MockMQTTClient (tests).
class IMQTTClient {
public:
    virtual ~IMQTTClient() = default;
    virtual void connect(const std::string& host, int port) = 0;
    virtual void disconnect() = 0;
    virtual void publish(const std::string& topic, const std::string& message) = 0;
};

// Thin wrapper around libmosquitto.
class MQTTClient : public IMQTTClient {
public:
    MQTTClient();
    ~MQTTClient() override;

    void connect(const std::string& host, int port) override;
    void disconnect() override;
    void publish(const std::string& topic, const std::string& message) override;

private:
    mosquitto* mosq_;
};