#pragma once

#include "virtual_device.hpp"

class VirtualLight : public VirtualDevice {
public:
    // Inherit the VirtualDevice constructor directly.
    using VirtualDevice::VirtualDevice;

    // Set initial states according to capabilities declared in device_models.json.
    // Must be called right after construction (done by DeviceEngine::load_from_json).
    void init_states();

    void update_state(const Event& event) override;
};
