#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "protocol_client.hpp"

class AdapterManager {
public:
    static AdapterManager& instance();

    AdapterManager();

    void register_client(const std::shared_ptr<ProtocolClient>& client);
    void unregister_client(AdapterProtocol protocol);
    void unregister_device(std::string device_id);

    bool has_client(AdapterProtocol protocol) const;
    std::shared_ptr<ProtocolClient> get_client(AdapterProtocol protocol) const;

    void connect(AdapterProtocol protocol, const std::string& endpoint);
    void disconnect(AdapterProtocol protocol);

    void connect_all(const std::string& rest_endpoint,
                     const std::string& mqtt_endpoint);
    void disconnect_all();

    void send_message(AdapterProtocol protocol,
                      const std::string& channel,
                      const std::string& payload);

    std::vector<SimulatedMessage> messages(AdapterProtocol protocol) const;
    void clear_messages(AdapterProtocol protocol);

private:
    void refresh_rest_servers();

    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<ProtocolClient>> clients_;
};