#pragma once

#include <string>
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
    ProtocolClient(): connected_(false) {}

    virtual ~ProtocolClient() = default;

    virtual AdapterProtocol protocol() const = 0;

    virtual void connect(const std::string& endpoint) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;

    virtual void send(const std::string& channel, const std::string& payload) = 0;

    virtual std::vector<SimulatedMessage> messages() const = 0;
    virtual void clear_messages() = 0;

protected:
    bool connected_;
};
