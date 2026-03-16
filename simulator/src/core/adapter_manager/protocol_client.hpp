#pragma once

#include <string>
#include <utility>
#include <vector>

enum class AdapterProtocol {
    Rest,
    Ws,
    Mqtt,
};

std::string to_string(AdapterProtocol protocol);

struct SimulatedMessage {
    AdapterProtocol protocol;
    std::string     channel;
    std::string     payload;
};

class ProtocolClient {
public:
    explicit ProtocolClient(std::string device_id = "")
        : connected_(false), device_id_(std::move(device_id)) {}

    virtual ~ProtocolClient() = default;

    const std::string& device_id() const { return device_id_; }
    void set_device_id(std::string device_id) { device_id_ = std::move(device_id); }

    virtual AdapterProtocol protocol() const = 0;

    virtual void connect(const std::string& endpoint) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;

    virtual void send(const std::string& channel, const std::string& payload) = 0;

    virtual std::vector<SimulatedMessage> messages() const = 0;
    virtual void clear_messages() = 0;

protected:
    bool connected_;
    std::string device_id_;
};
