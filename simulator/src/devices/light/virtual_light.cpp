#include "light/virtual_light.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

void VirtualLight::init_states() {
    set_state("on", "false");
    if (has_capability("brightness"))
        set_state("brightness", "100");
    if (has_capability("color"))
        set_state("color", "#ffffff");
}

void VirtualLight::update_state(const Event& event) {
    try {
        apply_state_payload(event.payload);
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualLight] Invalid payload for " << id() << ": " << ex.what() << "\n";
        return;
    }

    publish_state();
}
