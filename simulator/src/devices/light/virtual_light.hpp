#pragma once

#include "virtual_device.hpp"

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
};
