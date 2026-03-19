#include "virtual_device.hpp"

#include <algorithm>
#include <unordered_set>

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

bool VirtualDevice::has_capability_alias(const std::string& alias) const {
    return model_->capability_aliases.find(alias) != model_->capability_aliases.end();
}

bool VirtualDevice::has_available_event(const std::string& event_type) const {
    const auto& events = model_->available_events;
    return std::find(events.begin(), events.end(), event_type) != events.end();
}

bool VirtualDevice::is_known_variable(const std::string& key) const {
    return !resolve_state_key(key).empty();
}

std::string VirtualDevice::get_state(const std::string& key) const {
    auto it = states_.find(key);
    return it != states_.end() ? it->second : "";
}

void VirtualDevice::set_state(const std::string& key, const std::string& value) {
    states_[key] = value;
}

std::string VirtualDevice::state_key_for_capability(const std::string& capability) const {
    return capability;
}

std::vector<std::string> VirtualDevice::accepted_keys_for_capability(const std::string& capability) const {
    std::vector<std::string> keys;
    std::unordered_set<std::string> seen;

    auto push_unique = [&keys, &seen](const std::string& key) {
        if (key.empty()) {
            return;
        }

        if (seen.insert(key).second) {
            keys.push_back(key);
        }
    };

    push_unique(state_key_for_capability(capability));
    push_unique(capability);

    for (const auto& [alias, canonical_capability] : model_->capability_aliases) {
        if (canonical_capability == capability) {
            push_unique(alias);
        }
    }

    return keys;
}

std::string VirtualDevice::resolve_state_key(const std::string& key) const {
    if (key.empty()) {
        return "";
    }

    // Keep already-initialized state keys as-is.
    if (states_.find(key) != states_.end()) {
        return key;
    }

    if (has_capability(key)) {
        return state_key_for_capability(key);
    }

    const auto alias_it = model_->capability_aliases.find(key);
    if (alias_it != model_->capability_aliases.end()) {
        return state_key_for_capability(alias_it->second);
    }

    // Some state keys (e.g. "on") differ from capabilities (e.g. "on_off").
    for (const auto& capability : model_->capabilities) {
        if (state_key_for_capability(capability) == key) {
            return key;
        }
    }

    return "";
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
        const std::string resolved_key = resolve_state_key(key);
        if (resolved_key.empty()) {
            continue;
        }

        if (value.is_string()) {
            set_state(resolved_key, value.get<std::string>());
            continue;
        }

        set_state(resolved_key, value.dump());
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
