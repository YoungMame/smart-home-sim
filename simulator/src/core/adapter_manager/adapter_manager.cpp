#include "adapter_manager.hpp"

#include <stdexcept>
#include <utility>

#include "protocols/mqtt/mqtt_client.hpp"
#include "protocols/rest/rest_client.hpp"
#include "protocols/ws/ws_client.hpp"

AdapterManager::AdapterManager() {
    register_client(std::make_shared<RestClient>());
    register_client(std::make_shared<WsClient>());
    register_client(std::make_shared<MQTTClient>());
}

void AdapterManager::register_client(const std::shared_ptr<ProtocolClient>& client) {
    if (!client) {
        throw std::invalid_argument("Cannot register a null protocol client");
    }

    std::scoped_lock lock(mutex_);
    clients_[client->protocol()] = client;
}

void AdapterManager::unregister_client(AdapterProtocol protocol) {
    std::scoped_lock lock(mutex_);
    clients_.erase(protocol);
}

bool AdapterManager::has_client(AdapterProtocol protocol) const {
    std::scoped_lock lock(mutex_);
    return clients_.find(protocol) != clients_.end();
}

std::shared_ptr<ProtocolClient> AdapterManager::get_client(AdapterProtocol protocol) const {
    std::scoped_lock lock(mutex_);
    const auto it = clients_.find(protocol);
    if (it == clients_.end()) {
        throw std::runtime_error("No registered client for protocol: " + to_string(protocol));
    }

    return it->second;
}

void AdapterManager::connect(AdapterProtocol protocol, const std::string& endpoint) {
    get_client(protocol)->connect(endpoint);
}

void AdapterManager::disconnect(AdapterProtocol protocol) {
    get_client(protocol)->disconnect();
}

void AdapterManager::connect_all(const std::string& rest_endpoint,
                                 const std::string& ws_endpoint,
                                 const std::string& mqtt_endpoint) {
    connect(AdapterProtocol::Rest, rest_endpoint);
    connect(AdapterProtocol::Ws, ws_endpoint);
    connect(AdapterProtocol::Mqtt, mqtt_endpoint);
}

void AdapterManager::disconnect_all() {
    if (has_client(AdapterProtocol::Rest)) {
        disconnect(AdapterProtocol::Rest);
    }

    if (has_client(AdapterProtocol::Ws)) {
        disconnect(AdapterProtocol::Ws);
    }

    if (has_client(AdapterProtocol::Mqtt)) {
        disconnect(AdapterProtocol::Mqtt);
    }
}

void AdapterManager::send_message(AdapterProtocol protocol,
                                  const std::string& channel,
                                  const std::string& payload) {
    get_client(protocol)->send(channel, payload);
}

std::vector<SimulatedMessage> AdapterManager::messages(AdapterProtocol protocol) const {
    return get_client(protocol)->messages();
}

void AdapterManager::clear_messages(AdapterProtocol protocol) {
    get_client(protocol)->clear_messages();
}
