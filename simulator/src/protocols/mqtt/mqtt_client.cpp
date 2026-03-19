#include "mqtt_client.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "scheduler/event_scheduler.hpp"

namespace {

std::string normalize_endpoint(std::string endpoint) {
    constexpr const char* mqtt_scheme = "mqtt://";
    constexpr const char* tcp_scheme = "tcp://";

    if (endpoint.rfind(mqtt_scheme, 0) == 0) {
        endpoint.erase(0, std::char_traits<char>::length(mqtt_scheme));
    } else if (endpoint.rfind(tcp_scheme, 0) == 0) {
        endpoint.erase(0, std::char_traits<char>::length(tcp_scheme));
    }

    return endpoint;
}

std::vector<std::string> split_topic(const std::string& topic) {
    std::vector<std::string> parts;
    std::stringstream ss(topic);
    std::string item;

    while (std::getline(ss, item, '/')) {
        parts.push_back(item);
    }

    return parts;
}

std::string payload_to_string(const mosquitto_message* msg) {
    if (!msg || !msg->payload || msg->payloadlen <= 0) {
        return "{}";
    }

    return std::string(static_cast<const char*>(msg->payload), static_cast<std::size_t>(msg->payloadlen));
}

} // namespace

MQTTClient::MQTTClient() {
    set_device_id("mqtt-global");
    mosq_ = mosquitto_new(nullptr, true, nullptr);
    if (!mosq_)
        throw std::runtime_error("Failed to create MQTT client");

    mosquitto_user_data_set(mosq_, this);
    mosquitto_connect_callback_set(mosq_, &MQTTClient::on_connect_static);
    mosquitto_message_callback_set(mosq_, &MQTTClient::on_message_static);
}

MQTTClient::~MQTTClient() {
    try {
        disconnect();
    } catch (...) {
    }

    if (mosq_)
        mosquitto_destroy(mosq_);
}

AdapterProtocol MQTTClient::protocol() const {
    return AdapterProtocol::Mqtt;
}

void MQTTClient::connect(const std::string& endpoint) {
    const std::string normalized = normalize_endpoint(endpoint);

    const std::size_t separator_pos = normalized.rfind(':');
    if (separator_pos == std::string::npos) {
        connect(normalized, 1883);
        return;
    }

    const std::string host = normalized.substr(0, separator_pos);
    const std::string port_str = normalized.substr(separator_pos + 1);
    const int port = std::stoi(port_str);
    connect(host, port);
}

void MQTTClient::connect(const std::string& host, int port) {
    int ret = mosquitto_connect(mosq_, host.c_str(), port, 60);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to connect to MQTT broker: " + std::string(mosquitto_strerror(ret)));

    ret = mosquitto_loop_start(mosq_);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to start MQTT loop: " + std::string(mosquitto_strerror(ret)));

    {
        std::scoped_lock lock(mutex_);
        loop_started_ = true;
    }

    std::scoped_lock lock(mutex_);
    connected_ = true;
}

void MQTTClient::disconnect() {
    int ret = mosquitto_disconnect(mosq_);
    if (ret != MOSQ_ERR_NO_CONN && ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to disconnect from MQTT broker: " + std::string(mosquitto_strerror(ret)));

    bool should_stop_loop = false;
    {
        std::scoped_lock lock(mutex_);
        should_stop_loop = loop_started_;
        loop_started_ = false;
        connected_ = false;
    }

    if (should_stop_loop) {
        mosquitto_loop_stop(mosq_, true);
    }
}

bool MQTTClient::is_connected() const {
    std::scoped_lock lock(mutex_);
    return connected_;
}

void MQTTClient::send(const std::string& topic, const std::string& payload) {
    publish(topic, payload);
}

void MQTTClient::subscribe(const std::string& topic) {
    if (!is_connected()) {
        throw std::runtime_error("MQTT client is not connected");
    }

    std::cout << "[MQTTClient] Subscribing to topic '" << topic << "'\n";

    int ret = mosquitto_subscribe(mosq_, nullptr, topic.c_str(), 0);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to subscribe to topic: " + std::string(mosquitto_strerror(ret)));
}

void MQTTClient::unsubscribe(const std::string& topic) {
    if (!is_connected()) {
        std::cerr << "[MQTTClient] Cannot unsubscribe from '" << topic << "': not connected\n";
        return;
    }

    std::cout << "[MQTTClient] Unsubscribing from topic '" << topic << "'\n";

    int ret = mosquitto_unsubscribe(mosq_, nullptr, topic.c_str());
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error("Failed to unsubscribe from topic: " + std::string(mosquitto_strerror(ret)));
}

void MQTTClient::publish(const std::string& topic, const std::string& message) {
    if (!is_connected()) {
        throw std::runtime_error("MQTT client is not connected");
    }

    std::cout << "[MQTTClient] Publishing to topic '" << topic << "': " << message << "\n";

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

void MQTTClient::on_connect_static(struct mosquitto* /*mosq*/, void* obj, int rc) {
    auto* self = static_cast<MQTTClient*>(obj);
    if (self) {
        self->on_connect(rc);
    }
}

void MQTTClient::on_message_static(struct mosquitto* /*mosq*/, void* obj, const struct mosquitto_message* msg) {
    auto* self = static_cast<MQTTClient*>(obj);
    if (self) {
        self->on_message(msg);
    }
}

void MQTTClient::on_connect(int rc) {
    if (rc != 0) {
        std::cerr << "[MQTTClient] Broker connect callback error rc=" << rc << "\n";
    }
}

void MQTTClient::on_message(const struct mosquitto_message* msg) {
    if (!msg || !msg->topic) {
        return;
    }

    const std::string topic(msg->topic);
    const std::vector<std::string> parts = split_topic(topic);

    if (parts.empty() || parts[0] != "home") {
        return;
    }

    Event event;
    event.payload = payload_to_string(msg);
    event.scheduled_at = std::chrono::steady_clock::now();

    // New format: home/<room>/<id>
    if (parts.size() == 3) {
        event.device_id = parts[2];
        event.type = "state_change";
        EventScheduler::instance().schedule_event(std::move(event));
        return;
    }

    // Legacy format: home/<type>/<id>/<op>[/<subtype>]
    if (parts.size() < 4) {
        return;
    }

    event.device_id = parts[2];

    if (parts.size() == 4 && parts[3] == "set") {
        event.type = "state_change";
        EventScheduler::instance().schedule_event(std::move(event));
        return;
    }

    if (parts.size() == 5 && parts[3] == "event") {
        const std::string subtype = parts[4];
        event.type = (subtype.find('.') != std::string::npos)
            ? subtype
            : (parts[1] + "." + subtype);

        EventScheduler::instance().schedule_event(std::move(event));
    }
}