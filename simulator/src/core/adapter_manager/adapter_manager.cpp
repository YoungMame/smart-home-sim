#include "adapter_manager.hpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "core/device_engine/device_engine.hpp"
#include "protocols/mqtt/mqtt_client.hpp"
#include "protocols/rest/rest_server.hpp"
#include "protocols/rest/rest_servers_manager.hpp"
#include "protocols/ws/ws_client.hpp"
#include "protocols/ws/ws_servers_manager.hpp"

namespace {

std::vector<std::shared_ptr<RestServer>> collect_rest_servers(const std::vector<std::shared_ptr<ProtocolClient>>& clients) {
    std::vector<std::shared_ptr<RestServer>> servers;
    for (const auto& client : clients) {
        std::shared_ptr<RestServer> server = std::dynamic_pointer_cast<RestServer>(client);
        if (server) {
            servers.push_back(server);
        }
    }
    return servers;
}

std::vector<std::shared_ptr<WsClient>> collect_ws_clients(const std::vector<std::shared_ptr<ProtocolClient>>& clients) {
    std::vector<std::shared_ptr<WsClient>> servers;
    for (const auto& client : clients) {
        std::shared_ptr<WsClient> server = std::dynamic_pointer_cast<WsClient>(client);
        if (server) {
            servers.push_back(server);
        }
    }
    return servers;
}

} // namespace

AdapterManager& AdapterManager::instance() {
    static AdapterManager manager;
    return manager;
}

AdapterManager::AdapterManager() {
    rest_servers_manager_ = std::make_unique<RestServersManager>();
    ws_servers_manager_ = std::make_unique<WsServersManager>();
    register_client(std::make_shared<MQTTClient>());
}

void AdapterManager::register_client(const std::shared_ptr<ProtocolClient>& client) {
    if (!client) {
        throw std::invalid_argument("Cannot register a null protocol client");
    }

    std::scoped_lock lock(mutex_);
    if (client->protocol() == AdapterProtocol::Rest || client->protocol() == AdapterProtocol::Ws) {
        clients_.push_back(client);
        return;
    }

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

        if (protocol == AdapterProtocol::Rest) {
            rest_servers_manager_->clear();
        }
        if (protocol == AdapterProtocol::Ws) {
            ws_servers_manager_->clear();
        }
    }
}

void AdapterManager::unregister_device(std::string device_id) {
    std::vector<std::shared_ptr<ProtocolClient>> removed_clients;
    {
        std::scoped_lock lock(mutex_);
        rest_servers_manager_->remove_device(device_id);
        ws_servers_manager_->remove_device(device_id);

        auto keep_it = std::remove_if(clients_.begin(), clients_.end(),
            [&removed_clients, &device_id](const std::shared_ptr<ProtocolClient>& client) {
                if ((client->protocol() == AdapterProtocol::Rest ||
                     client->protocol() == AdapterProtocol::Ws) &&
                    client->device_id() == device_id) {
                    removed_clients.push_back(client);
                    return true;
                }

                return false;
            });
        clients_.erase(keep_it, clients_.end());
    }
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
    if (protocol == AdapterProtocol::Rest) {
        refresh_rest_servers(endpoint);

        std::vector<std::shared_ptr<RestServer>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_rest_servers(clients_);
        }

        if (servers.empty()) {
            throw std::runtime_error("No REST servers available: no virtual devices are loaded");
        }

        for (const auto& server : servers) {
            const auto endpoint_for_device = rest_servers_manager_->endpoint_for(server->device_id());
            if (!endpoint_for_device.has_value()) {
                throw std::runtime_error("No endpoint assigned for REST server device: " +
                                         server->device_id());
            }
            server->connect(*endpoint_for_device);
        }
        return;
    }

    if (protocol == AdapterProtocol::Ws) {
        refresh_ws_servers(endpoint);

        std::vector<std::shared_ptr<WsClient>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_ws_clients(clients_);
        }

        if (servers.empty()) {
            throw std::runtime_error("No WS clients available: no virtual devices are loaded");
        }

        for (const auto& server : servers) {
            const auto endpoint_for_device = ws_servers_manager_->endpoint_for(server->device_id());
            if (!endpoint_for_device.has_value()) {
                throw std::runtime_error("No endpoint assigned for WS client device: " +
                                         server->device_id());
            }
            server->connect(*endpoint_for_device);
        }
        return;
    }

    get_client(protocol)->connect(endpoint);
}

void AdapterManager::disconnect(AdapterProtocol protocol) {
    if (protocol == AdapterProtocol::Rest) {
        std::vector<std::shared_ptr<RestServer>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_rest_servers(clients_);
        }

        for (const auto& server : servers) {
            server->disconnect();
        }
        return;
    }

    if (protocol == AdapterProtocol::Ws) {
        std::vector<std::shared_ptr<WsClient>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_ws_clients(clients_);
        }

        for (const auto& server : servers) {
            server->disconnect();
        }
        return;
    }

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
    if (protocol == AdapterProtocol::Rest) {
        constexpr std::string_view separator = ":";
        const std::size_t split_pos = channel.find(separator);
        if (split_pos == std::string::npos || split_pos == 0 || split_pos + separator.size() >= channel.size()) {
            throw std::invalid_argument(
                "REST channel must be formatted as '<device_id>:<route>'");
        }

        const std::string device_id = channel.substr(0, split_pos);
        const std::string route = channel.substr(split_pos + separator.size());

        std::shared_ptr<RestServer> server;
        {
            std::scoped_lock lock(mutex_);
            const auto servers = collect_rest_servers(clients_);
            const auto it = std::find_if(servers.begin(), servers.end(),
                [&device_id](const std::shared_ptr<RestServer>& candidate) {
                    return candidate->device_id() == device_id;
                });
            if (it == servers.end()) {
                throw std::runtime_error("No REST server registered for device: " + device_id);
            }
            server = *it;
        }

        server->send(route, payload);
        return;
    }

    if (protocol == AdapterProtocol::Ws) {
        constexpr std::string_view separator = ":";
        const std::size_t split_pos = channel.find(separator);
        if (split_pos == std::string::npos || split_pos == 0 || split_pos + separator.size() >= channel.size()) {
            throw std::invalid_argument(
                "WS channel must be formatted as '<device_id>:<channel>'");
        }

        const std::string device_id = channel.substr(0, split_pos);
        const std::string ws_channel = channel.substr(split_pos + separator.size());

        std::shared_ptr<WsClient> server;
        {
            std::scoped_lock lock(mutex_);
            const auto servers = collect_ws_clients(clients_);
            const auto it = std::find_if(servers.begin(), servers.end(),
                [&device_id](const std::shared_ptr<WsClient>& candidate) {
                    return candidate->device_id() == device_id;
                });
            if (it == servers.end()) {
                throw std::runtime_error("No WS client registered for device: " + device_id);
            }
            server = *it;
        }

        server->send(ws_channel, payload);
        return;
    }

    get_client(protocol)->send(channel, payload);
}

std::vector<SimulatedMessage> AdapterManager::messages(AdapterProtocol protocol) const {
    if (protocol == AdapterProtocol::Rest) {
        std::vector<std::shared_ptr<RestServer>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_rest_servers(clients_);
        }

        std::vector<SimulatedMessage> all_messages;
        for (const auto& server : servers) {
            auto server_messages = server->messages();
            all_messages.insert(all_messages.end(), server_messages.begin(), server_messages.end());
        }

        return all_messages;
    }

    if (protocol == AdapterProtocol::Ws) {
        std::vector<std::shared_ptr<WsClient>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_ws_clients(clients_);
        }

        std::vector<SimulatedMessage> all_messages;
        for (const auto& server : servers) {
            auto server_messages = server->messages();
            all_messages.insert(all_messages.end(), server_messages.begin(), server_messages.end());
        }

        return all_messages;
    }

    return get_client(protocol)->messages();
}

void AdapterManager::clear_messages(AdapterProtocol protocol) {
    if (protocol == AdapterProtocol::Rest) {
        std::vector<std::shared_ptr<RestServer>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_rest_servers(clients_);
        }

        for (const auto& server : servers) {
            server->clear_messages();
        }
        return;
    }

    if (protocol == AdapterProtocol::Ws) {
        std::vector<std::shared_ptr<WsClient>> servers;
        {
            std::scoped_lock lock(mutex_);
            servers = collect_ws_clients(clients_);
        }

        for (const auto& server : servers) {
            server->clear_messages();
        }
        return;
    }

    get_client(protocol)->clear_messages();
}

void AdapterManager::refresh_rest_servers(const std::string& rest_base_endpoint) {
    const auto devices = DeviceEngine::instance().snapshot_devices();

    std::scoped_lock lock(mutex_);
    rest_servers_manager_->clear();

    clients_.erase(
        std::remove_if(clients_.begin(), clients_.end(),
            [](const std::shared_ptr<ProtocolClient>& client) {
                return client->protocol() == AdapterProtocol::Rest;
            }),
        clients_.end());

    for (const auto& device : devices) {
        (void)rest_servers_manager_->assign_endpoint(device->id(), rest_base_endpoint);
        clients_.push_back(std::make_shared<RestServer>(device->id()));
    }
}

void AdapterManager::refresh_ws_servers(const std::string& ws_base_endpoint) {
    const auto devices = DeviceEngine::instance().snapshot_devices();

    std::scoped_lock lock(mutex_);
    ws_servers_manager_->clear();

    clients_.erase(
        std::remove_if(clients_.begin(), clients_.end(),
            [](const std::shared_ptr<ProtocolClient>& client) {
                return client->protocol() == AdapterProtocol::Ws;
            }),
        clients_.end());

    for (const auto& device : devices) {
        (void)ws_servers_manager_->assign_endpoint(device->id(), ws_base_endpoint);
        clients_.push_back(std::make_shared<WsClient>(device->id()));
    }
}
