#include "rest_client.hpp"

#include <stdexcept>

namespace {

[[noreturn]] void throw_not_connected(AdapterProtocol protocol) {
    throw std::runtime_error("Client not connected for protocol: " + to_string(protocol));
}

} // namespace

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

SimulatedMessage RestClient::get(const std::string& route, const std::string& payload) {
    std::scoped_lock lock(mutex_);
    if (!connected_) {
        throw_not_connected(AdapterProtocol::Rest);
    }

    SimulatedMessage request{AdapterProtocol::Rest, route, payload};
    requests_.push_back(request);
    return request;
}

std::vector<SimulatedMessage> RestClient::requests() const {
    std::scoped_lock lock(mutex_);
    return requests_;
}

void RestClient::clear_requests() {
    std::scoped_lock lock(mutex_);
    requests_.clear();
}
