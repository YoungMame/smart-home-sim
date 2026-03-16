#include "rest_client.hpp"

#include <stdexcept>

namespace {

[[noreturn]] void throw_not_connected(AdapterProtocol protocol) {
    throw std::runtime_error("Client not connected for protocol: " + to_string(protocol));
}

} // namespace

AdapterProtocol RestClient::protocol() const {
    return AdapterProtocol::Rest;
}

void RestClient::connect(const std::string& endpoint) {
    std::scoped_lock lock(mutex_);
    endpoint_ = endpoint;
    connected_ = true;
}

void RestClient::disconnect() {
    std::scoped_lock lock(mutex_);
    connected_ = false;
}

bool RestClient::is_connected() const {
    std::scoped_lock lock(mutex_);
    return connected_;
}

void RestClient::send(const std::string& route, const std::string& payload) {
    std::scoped_lock lock(mutex_);
    if (!connected_) {
        throw_not_connected(protocol());
    }

    messages_.push_back({protocol(), route, payload});
}

std::vector<SimulatedMessage> RestClient::messages() const {
    std::scoped_lock lock(mutex_);
    return messages_;
}

void RestClient::clear_messages() {
    std::scoped_lock lock(mutex_);
    messages_.clear();
}
