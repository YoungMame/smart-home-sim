#include "thermostat/virtual_thermostat.hpp"

void VirtualThermostat::init_states() {
    set_state("temperature",        "20.0");
    set_state("target_temperature", "21.0");
}

void VirtualThermostat::update_state(const Event& event) {
    // TODO: parse event.payload (JSON) and call set_state.
    // Expected payload example: {"target_temperature": "22.5"}
    (void)event;
}
