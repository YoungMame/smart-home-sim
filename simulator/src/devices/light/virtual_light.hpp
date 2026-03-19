#pragma once

# include "virtual_device.hpp"
# include "event_engine/event_engine.hpp"

class VirtualLight : public VirtualDevice {
public:
    VirtualLight() = delete;
    ~VirtualLight() = default;

    VirtualLight(const VirtualLight&) = delete;
    VirtualLight& operator=(const VirtualLight&) = delete;

    // Inherit the VirtualDevice constructor directly.
    using VirtualDevice::VirtualDevice;

    // Set initial states according to capabilities declared in the model catalog.
    // Must be called right after construction (done by DeviceEngine).
    void init_states();

    void update_state(const Event& event) override;

    void register_event_handlers();

private:
    std::string state_key_for_capability(const std::string& capability) const override;
    void handle_light_turned_on(const Event& event);
    void handle_light_turned_off(const Event& event);
    void handle_light_brightness_changed(const Event& event);
    void handle_light_color_changed(const Event& event);
};
