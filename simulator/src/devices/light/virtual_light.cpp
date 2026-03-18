#include "light/virtual_light.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

void VirtualLight::register_event_handlers() {
    if (has_capability("on_off") && has_available_event("light.turned_on")) {
        EventEngine::instance().add_event_handler(id(), "light.turned_on", [this](const Event& event) {
            handle_light_turned_on(event);
        });
    }

    if (has_capability("on_off") && has_available_event("light.turned_off")) {
        EventEngine::instance().add_event_handler(id(), "light.turned_off", [this](const Event& event) {
            handle_light_turned_off(event);
        });
    }

    if (has_capability("brightness") && has_available_event("light.brightness_changed")) {
        EventEngine::instance().add_event_handler(id(), "light.brightness_changed", [this](const Event& event) {
            handle_light_brightness_changed(event);
        });
    }

    if (has_capability("color") && has_available_event("light.color_changed")) {
        EventEngine::instance().add_event_handler(id(), "light.color_changed", [this](const Event& event) {
            handle_light_color_changed(event);
        });
    }
}

void VirtualLight::init_states() {
    set_state("on", "false");
    register_event_handlers();

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

void VirtualLight::handle_light_turned_on(const Event& event) {
    (void)event;
    set_state("on", "true");
    publish_state();
}

void VirtualLight::handle_light_turned_off(const Event& event) {
    (void)event;
    set_state("on", "false");
    publish_state();
}

void VirtualLight::handle_light_brightness_changed(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        const std::string value = body.value("brightness", body.value("value", std::string("100")));
        set_state("brightness", value);
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualLight] Invalid payload for brightness_changed on " << id() << ": " << ex.what() << "\n";
    }
}

void VirtualLight::handle_light_color_changed(const Event& event) {
    try {
        const auto body = nlohmann::json::parse(event.payload.empty() ? "{}" : event.payload);
        const std::string value = body.value("color", body.value("value", std::string("#ffffff")));
        set_state("color", value);
        publish_state();
    } catch (const nlohmann::json::exception& ex) {
        std::cerr << "[VirtualLight] Invalid payload for color_changed on " << id() << ": " << ex.what() << "\n";
    }
}
