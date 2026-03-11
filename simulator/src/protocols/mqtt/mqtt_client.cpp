#include "mqtt_client.hpp"

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

void MQTTClient::connect(const std::string& host, int port) {
    int ret = mosquitto_connect(mosq_, host.c_str(), port, 60);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to connect to MQTT broker: " + std::string(mosquitto_strerror(ret)));
}

void MQTTClient::disconnect() {
    mosquitto_disconnect(mosq_);
}

void MQTTClient::publish(const std::string& topic, const std::string& message) {
    int ret = mosquitto_publish(mosq_, nullptr, topic.c_str(),
                               static_cast<int>(message.size()), message.c_str(), 0, false);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to publish message: " + std::string(mosquitto_strerror(ret)));
}