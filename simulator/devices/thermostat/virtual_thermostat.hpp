#pragma once

#include "virtual_device.hpp"

class VirtualThermostat : public VirtualDevice {
public:
    using VirtualDevice::VirtualDevice;

    // Set initial states: current temperature and target temperature.
    void init_states();

    void update_state(const Event& event) override;
};
