#include "adapter_manager.hpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "protocols/rest/rest_server.hpp"

AdapterManager& AdapterManager::instance() {
    static AdapterManager manager;
    return manager;
}

AdapterManager::AdapterManager() = default;

void AdapterManager::register_client(const std::shared_ptr<ProtocolClient>& client) {
    if (!client) {
        throw std::invalid_argument("Cannot register a null protocol client");
    }

    std::scoped_lock lock(mutex_);
    const auto it = std::find_if(clients_.begin(), clients_.end(),
        [&client](const std::shared_ptr<ProtocolClient>& existing) {
            return existing->protocol() == client->protocol();
        });

    if (it == clients_.end()) {
        clients_.push_back(client);
    } else {
        *it = client;
    }
}

void AdapterManager::unregister_client(AdapterProtocol protocol) {
    std::vector<std::shared_ptr<ProtocolClient>> removed_clients;
    {
        std::scoped_lock lock(mutex_);
        auto keep_it = std::remove_if(clients_.begin(), clients_.end(),
            [&removed_clients, protocol](const std::shared_ptr<ProtocolClient>& client) {
                if (client->protocol() == protocol) {
                    removed_clients.push_back(client);
                    return true;
                }

                return false;
            });
        clients_.erase(keep_it, clients_.end());

    }
}

void AdapterManager::unregister_device(std::string device_id) {
    (void)device_id;
    // REST is a shared singleton client; deleting one device must not unregister transport.
}

bool AdapterManager::has_client(AdapterProtocol protocol) const {
    std::scoped_lock lock(mutex_);
    return std::any_of(clients_.begin(), clients_.end(),
        [protocol](const std::shared_ptr<ProtocolClient>& client) {
            return client->protocol() == protocol;
        });
}

std::shared_ptr<ProtocolClient> AdapterManager::get_client(AdapterProtocol protocol) const {
    std::scoped_lock lock(mutex_);
    const auto it = std::find_if(clients_.begin(), clients_.end(),
        [protocol](const std::shared_ptr<ProtocolClient>& client) {
            return client->protocol() == protocol;
        });

    if (it == clients_.end()) {
        throw std::runtime_error("No registered client for protocol: " + to_string(protocol));
    }

    return *it;
}

void AdapterManager::connect(AdapterProtocol protocol, const std::string& endpoint) {
    get_client(protocol)->connect(endpoint);
}

void AdapterManager::disconnect(AdapterProtocol protocol) {
    get_client(protocol)->disconnect();
}

void AdapterManager::connect_all(const std::string& rest_endpoint,
                                 const std::string& mqtt_endpoint) {
    connect(AdapterProtocol::Rest, rest_endpoint);
    connect(AdapterProtocol::Mqtt, mqtt_endpoint);
}

void AdapterManager::disconnect_all() {
    if (has_client(AdapterProtocol::Rest)) {
        disconnect(AdapterProtocol::Rest);
    }

    if (has_client(AdapterProtocol::Mqtt)) {
        disconnect(AdapterProtocol::Mqtt);
    }
}

void AdapterManager::send_message(AdapterProtocol protocol,
                                  const std::string& channel,
                                  const std::string& payload) {
    if (protocol == AdapterProtocol::Rest) {
        constexpr std::string_view separator = ":";
        const std::size_t split_pos = channel.find(separator);

        // Backward compatibility: allow both '<device_id>:<route>' and direct '<route>'.
        const std::string route = (split_pos != std::string::npos && split_pos + separator.size() < channel.size())
            ? channel.substr(split_pos + separator.size())
            : channel;

        get_client(protocol)->send(route, payload);
        return;
    }

    get_client(protocol)->send(channel, payload);
}

std::vector<SimulatedMessage> AdapterManager::messages(AdapterProtocol protocol) const {
    return get_client(protocol)->messages();
}

void AdapterManager::clear_messages(AdapterProtocol protocol) {
    get_client(protocol)->clear_messages();
}

