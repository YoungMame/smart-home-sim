#pragma once

#include "virtual_device.hpp"

class VirtualThermostat : public VirtualDevice {
public:
    VirtualThermostat() = delete;
    ~VirtualThermostat() = default;

    VirtualThermostat(const VirtualThermostat&) = delete;
    VirtualThermostat& operator=(const VirtualThermostat&) = delete;

    using VirtualDevice::VirtualDevice;

    // Set initial states: current temperature and target temperature.
    void init_states();

    void update_state(const Event& event) override;

    void register_event_handlers();

private:
    void handle_thermostat_temperature_changed(const Event& event);
    void handle_thermostat_setpoint_changed(const Event& event);
    void handle_thermostat_mode_changed(const Event& event);
    void handle_thermostat_humidity_changed(const Event& event);
};
