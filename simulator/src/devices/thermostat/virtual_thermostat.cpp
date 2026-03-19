#include "thermostat/virtual_thermostat.hpp"

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

void VirtualThermostat::register_event_handlers() {
    if (has_available_event("thermostat.temperature_changed")) {
        EventEngine::instance().add_event_handler(id(), "thermostat.temperature_changed", [this](const Event& event) {
            handle_thermostat_temperature_changed(event);
        });
    }

    if (has_available_event("thermostat.setpoint_changed")) {
        EventEngine::instance().add_event_handler(id(), "thermostat.setpoint_changed", [this](const Event& event) {
            handle_thermostat_setpoint_changed(event);
        });
    }

    if (has_available_event("thermostat.mode_changed")) {
        EventEngine::instance().add_event_handler(id(), "thermostat.mode_changed", [this](const Event& event) {
            handle_thermostat_mode_changed(event);
        });
    }

    if (has_available_event("thermostat.humidity_changed")) {
        EventEngine::instance().add_event_handler(id(), "thermostat.humidity_changed", [this](const Event& event) {
            handle_thermostat_humidity_changed(event);
        });
    }
}

void VirtualThermostat::init_states() {
    set_state("temperature",        "20.0");
    set_state("target_temperature", "21.0");
    register_event_handlers();
}

void VirtualThermostat::update_state(const Event& event) {
    try {
        apply_state_payload(event.payload);
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualThermostat] Invalid payload for " << id() << ": " << ex.what() << "\n";
        return;
    }

    publish_state();
}

void VirtualThermostat::handle_thermostat_temperature_changed(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("temperature", first_present_value(body, accepted_keys_for_capability("temperature"), "value", "20.0"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualThermostat] Invalid payload for temperature_changed on " << id() << ": " << ex.what() << "\n";
    }
}

void VirtualThermostat::handle_thermostat_setpoint_changed(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("target_temperature", first_present_value(body, accepted_keys_for_capability("target_temperature"), "value", "21.0"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualThermostat] Invalid payload for setpoint_changed on " << id() << ": " << ex.what() << "\n";
    }
}

void VirtualThermostat::handle_thermostat_mode_changed(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("mode", first_present_value(body, {"mode"}, "value", "auto"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualThermostat] Invalid payload for mode_changed on " << id() << ": " << ex.what() << "\n";
    }
}

void VirtualThermostat::handle_thermostat_humidity_changed(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        set_state("humidity", first_present_value(body, {"humidity"}, "value", "50.0"));
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualThermostat] Invalid payload for humidity_changed on " << id() << ": " << ex.what() << "\n";
    }
}
