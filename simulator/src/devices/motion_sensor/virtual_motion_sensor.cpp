#include "motion_sensor/virtual_motion_sensor.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

#include "event_engine/event_engine.hpp"

namespace {
std::string to_state_value(const nlohmann::json& value) {
    if (value.is_string()) {
        return value.get<std::string>();
    }

    return value.dump();
}

std::string first_present_value(const nlohmann::json& body,
                                const std::vector<std::string>& keys,
                                const std::string& fallback_key,
                                const std::string& default_value) {
    for (const auto& key : keys) {
        if (body.contains(key)) {
            return to_state_value(body.at(key));
        }
    }

    if (!fallback_key.empty() && body.contains(fallback_key)) {
        return to_state_value(body.at(fallback_key));
    }

    return default_value;
}
}

void VirtualMotionSensor::register_event_handlers() {
    if (has_available_event("motion.motion_detected")) {
        EventEngine::instance().add_event_handler(id(), "motion.motion_detected", [this](const Event& event) {
            handle_motion_detected(event);
        });
    }

    if (has_available_event("motion.motion_cleared")) {
        EventEngine::instance().add_event_handler(id(), "motion.motion_cleared", [this](const Event& event) {
            handle_motion_cleared(event);
        });
    }

    if (has_available_event("motion.presence_detected")) {
        EventEngine::instance().add_event_handler(id(), "motion.presence_detected", [this](const Event& event) {
            handle_presence_detected(event);
        });
    }

    if (has_available_event("motion.presence_cleared")) {
        EventEngine::instance().add_event_handler(id(), "motion.presence_cleared", [this](const Event& event) {
            handle_presence_cleared(event);
        });
    }
}

void VirtualMotionSensor::init_states() {
    set_state("motion", "false");
    set_state("presence", "false");
    register_event_handlers();
}

void VirtualMotionSensor::update_state(const Event& event) {
    try {
        apply_state_payload(event.payload);
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualMotionSensor] Invalid payload for " << id() << ": " << ex.what() << "\n";
        return;
    }

    publish_state();
}

void VirtualMotionSensor::handle_motion_detected(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("motion", first_present_value(body, accepted_keys_for_capability("motion"), "value", "true"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualMotionSensor] Invalid payload for motion_detected on " << id() << ": " << ex.what() << "\n";
    }
}

void VirtualMotionSensor::handle_motion_cleared(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("motion", first_present_value(body, accepted_keys_for_capability("motion"), "value", "false"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualMotionSensor] Invalid payload for motion_cleared on " << id() << ": " << ex.what() << "\n";
    }
}

void VirtualMotionSensor::handle_presence_detected(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("presence", first_present_value(body, accepted_keys_for_capability("presence"), "value", "true"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualMotionSensor] Invalid payload for presence_detected on " << id() << ": " << ex.what() << "\n";
    }
}

void VirtualMotionSensor::handle_presence_cleared(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("presence", first_present_value(body, accepted_keys_for_capability("presence"), "value", "false"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualMotionSensor] Invalid payload for presence_cleared on " << id() << ": " << ex.what() << "\n";
    }
}
