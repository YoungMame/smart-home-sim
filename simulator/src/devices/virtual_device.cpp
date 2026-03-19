#include "virtual_device.hpp"

#include <algorithm>

#include <nlohmann/json.hpp>

#include "core/adapter_manager/adapter_manager.hpp"
#include "protocols/mqtt/mqtt_client.hpp"
#include "protocols/rest/rest_server.hpp"

using json = nlohmann::json;

VirtualDevice::VirtualDevice(std::string id,
                              std::string label,
                              std::string room,
                              const VirtualDeviceModel* model)
    : id_(std::move(id))
    , label_(std::move(label))
    , room_(std::move(room))
    , model_(model)
{
    if (!model_) {
        throw std::invalid_argument("Model pointer cannot be null");
    }

    auto client = build_protocol_client_();
    if (client) {
        protocol_client_ = client.get();
        AdapterManager::instance().register_client(client);
    }
}

std::shared_ptr<ProtocolClient> VirtualDevice::build_protocol_client_() {
    if (model_->protocol == "mqtt") {
        if (AdapterManager::instance().has_client(AdapterProtocol::Mqtt)) {
            return AdapterManager::instance().get_client(AdapterProtocol::Mqtt);
        }

        return std::make_shared<MQTTClient>();
    }

    if (model_->protocol == "rest") {
        if (AdapterManager::instance().has_client(AdapterProtocol::Rest)) {
            return AdapterManager::instance().get_client(AdapterProtocol::Rest);
        }

        return std::make_shared<RestServer>("rest-global");
    }

    throw std::runtime_error("Unsupported protocol for device: " + model_->protocol);
}

VirtualDevice::~VirtualDevice() {
    // Cleanup is coordinated by DeviceEngine to avoid singleton teardown order issues.
};

bool VirtualDevice::has_capability(const std::string& cap) const {
    const auto& caps = model_->capabilities;
    return std::find(caps.begin(), caps.end(), cap) != caps.end();
}

bool VirtualDevice::has_available_event(const std::string& event_type) const {
    const auto& events = model_->available_events;
    return std::find(events.begin(), events.end(), event_type) != events.end();
}

std::string VirtualDevice::get_state(const std::string& key) const {
    auto it = states_.find(key);
    return it != states_.end() ? it->second : "";
}

void VirtualDevice::set_state(const std::string& key, const std::string& value) {
    states_[key] = value;
}

void VirtualDevice::apply_state_payload(const std::string& payload) {
    if (payload.empty()) {
        return;
    }

    const json body = json::parse(payload);
    if (!body.is_object()) {
        throw json::type_error::create(302, "state payload must be a JSON object", &body);
    }

    for (const auto& [key, value] : body.items()) {
        if (value.is_string()) {
            set_state(key, value.get<std::string>());
            continue;
        }

        set_state(key, value.dump());
    }
}

std::string VirtualDevice::state_topic() const {
    return "home/" + room_ + "/" + id_ + "/state";
}

std::string VirtualDevice::command_topic() const {
    return "home/" + room_ + "/" + id_;
}

void VirtualDevice::publish_state() const {
    if (!protocol_client_ || !protocol_client_->is_connected()) return;

    json payload = json::object();
    for (const auto& [k, v] : states_)
        payload[k] = v;

    protocol_client_->send(state_topic(), payload.dump());
}
