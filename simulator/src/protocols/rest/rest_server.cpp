#include "rest_server.hpp"

#include <stdexcept>
#include <utility>

namespace {

[[noreturn]] void throw_not_connected(AdapterProtocol protocol) {
    throw std::runtime_error("Client not connected for protocol: " + to_string(protocol));
}

} // namespace

RestServer::RestServer(std::string device_id)
    : ProtocolClient(std::move(device_id)) {}

RestServer::~RestServer() {
    disconnect();
};

AdapterProtocol RestServer::protocol() const {
    return AdapterProtocol::Rest;
}

void RestServer::connect(const std::string& endpoint) {
    if (endpoint.empty()) {
        throw std::invalid_argument("REST endpoint cannot be empty");
    }

    std::scoped_lock lock(mutex_);
    endpoint_ = endpoint;
    client_.connect(endpoint_);
    connected_ = true;
}

void RestServer::disconnect() {
    std::scoped_lock lock(mutex_);
    if (!connected_) {
        return;
    }

    client_.disconnect();
    connected_ = false;
}

bool RestServer::is_connected() const {
    std::scoped_lock lock(mutex_);
    return connected_;
}

void RestServer::send(const std::string& route, const std::string& payload) {
    std::scoped_lock lock(mutex_);
    if (!connected_) {
        throw_not_connected(protocol());
    }

    const SimulatedMessage request = client_.get(route, payload);
    messages_.push_back(request);
}

std::vector<SimulatedMessage> RestServer::messages() const {
    std::scoped_lock lock(mutex_);
    return messages_;
}

void RestServer::clear_messages() {
    std::scoped_lock lock(mutex_);
    messages_.clear();
    client_.clear_requests();
}
