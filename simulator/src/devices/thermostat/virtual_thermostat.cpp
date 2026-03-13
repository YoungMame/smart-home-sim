#include "thermostat/virtual_thermostat.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

void VirtualThermostat::init_states() {
    set_state("temperature",        "20.0");
    set_state("target_temperature", "21.0");
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
