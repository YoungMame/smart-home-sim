#include "virtual_device.hpp"

#include <algorithm>

#include <nlohmann/json.hpp>

#include "../protocols/mqtt/mqtt_client.hpp"

using json = nlohmann::json;

VirtualDevice::VirtualDevice(std::string id,
                              std::string label,
                              std::string room,
                              const VirtualDeviceModel* model)
    : id_(std::move(id))
    , label_(std::move(label))
    , room_(std::move(room))
    , model_(model)
{}

bool VirtualDevice::has_capability(const std::string& cap) const {
    const auto& caps = model_->capabilities;
    return std::find(caps.begin(), caps.end(), cap) != caps.end();
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
    return "home/" + model_->type + "/" + id_ + "/state";
}

void VirtualDevice::publish_state() const {
    if (!mqtt_client_) return;

    json payload = json::object();
    for (const auto& [k, v] : states_)
        payload[k] = v;

    mqtt_client_->publish(state_topic(), payload.dump());
}
