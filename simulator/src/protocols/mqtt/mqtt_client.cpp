#include "mqtt_client.hpp"

#include <mutex>
#include <stdexcept>
#include <string>

MQTTClient::MQTTClient() {
    mosq_ = mosquitto_new(nullptr, true, nullptr);
    if (!mosq_)
        throw std::runtime_error("Failed to create MQTT client");
}

MQTTClient::~MQTTClient() {
    if (mosq_)
        mosquitto_destroy(mosq_);
}

AdapterProtocol MQTTClient::protocol() const {
    return AdapterProtocol::Mqtt;
}

void MQTTClient::connect(const std::string& endpoint) {
    const std::size_t separator_pos = endpoint.rfind(':');
    if (separator_pos == std::string::npos) {
        connect(endpoint, 1883);
        return;
    }

    const std::string host = endpoint.substr(0, separator_pos);
    const std::string port_str = endpoint.substr(separator_pos + 1);
    const int port = std::stoi(port_str);
    connect(host, port);
}

void MQTTClient::connect(const std::string& host, int port) {
    int ret = mosquitto_connect(mosq_, host.c_str(), port, 60);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to connect to MQTT broker: " + std::string(mosquitto_strerror(ret)));

    std::scoped_lock lock(mutex_);
    connected_ = true;
}

void MQTTClient::disconnect() {
    mosquitto_disconnect(mosq_);

    std::scoped_lock lock(mutex_);
    connected_ = false;
}

bool MQTTClient::is_connected() const {
    std::scoped_lock lock(mutex_);
    return connected_;
}

void MQTTClient::send(const std::string& topic, const std::string& payload) {
    publish(topic, payload);
}

void MQTTClient::publish(const std::string& topic, const std::string& message) {
    if (!is_connected()) {
        throw std::runtime_error("MQTT client is not connected");
    }

    int ret = mosquitto_publish(mosq_, nullptr, topic.c_str(),
                               static_cast<int>(message.size()), message.c_str(), 0, false);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to publish message: " + std::string(mosquitto_strerror(ret)));

    std::scoped_lock lock(mutex_);
    messages_.push_back({protocol(), topic, message});
}

std::vector<SimulatedMessage> MQTTClient::messages() const {
    std::scoped_lock lock(mutex_);
    return messages_;
}

void MQTTClient::clear_messages() {
    std::scoped_lock lock(mutex_);
    messages_.clear();
}