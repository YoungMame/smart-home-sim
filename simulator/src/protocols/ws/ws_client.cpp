#include "ws_client.hpp"

#include <stdexcept>
#include <utility>

#include "core/adapter_manager/adapter_manager.hpp"

namespace {

[[noreturn]] void throw_not_connected(AdapterProtocol protocol) {
    throw std::runtime_error("Client not connected for protocol: " + to_string(protocol));
}

} // namespace

WsClient::WsClient(std::string device_id)
    : ProtocolClient(std::move(device_id)) {}

WsClient::~WsClient() {
    try {
        disconnect();
    } catch (...) {
    }

    try {
        if (!device_id().empty()) {
            AdapterManager::instance().unregister_device(device_id());
        } else {
            AdapterManager::instance().unregister_client(AdapterProtocol::Ws);
        }
    } catch (...) {
    }
}

AdapterProtocol WsClient::protocol() const {
    return AdapterProtocol::Ws;
}

void WsClient::connect(const std::string& endpoint) {
    std::scoped_lock lock(mutex_);
    endpoint_ = endpoint;
    connected_ = true;
}

void WsClient::disconnect() {
    std::scoped_lock lock(mutex_);
    connected_ = false;
}

bool WsClient::is_connected() const {
    std::scoped_lock lock(mutex_);
    return connected_;
}

void WsClient::send(const std::string& channel, const std::string& payload) {
    std::scoped_lock lock(mutex_);
    if (!connected_) {
        throw_not_connected(protocol());
    }

    messages_.push_back({protocol(), channel, payload});
}

std::vector<SimulatedMessage> WsClient::messages() const {
    std::scoped_lock lock(mutex_);
    return messages_;
}

void WsClient::clear_messages() {
    std::scoped_lock lock(mutex_);
    messages_.clear();
}
