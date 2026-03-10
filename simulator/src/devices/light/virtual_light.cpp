#include "light/virtual_light.hpp"

void VirtualLight::init_states() {
    set_state("on", "false");
    if (has_capability("brightness"))
        set_state("brightness", "100");
    if (has_capability("color"))
        set_state("color", "#ffffff");
}

void VirtualLight::update_state(const Event& event) {
    // TODO: parse event.payload (JSON) to extract key/value pairs and call set_state.
    // Expected payload example: {"on": "true"} or {"brightness": "75"}
    (void)event;
}
