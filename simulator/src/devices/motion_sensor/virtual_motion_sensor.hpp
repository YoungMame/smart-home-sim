#pragma once

#include "virtual_device.hpp"

class VirtualMotionSensor : public VirtualDevice {
public:
    VirtualMotionSensor() = delete;
    ~VirtualMotionSensor() = default;

    VirtualMotionSensor(const VirtualMotionSensor&) = delete;
    VirtualMotionSensor& operator=(const VirtualMotionSensor&) = delete;

    using VirtualDevice::VirtualDevice;

    void init_states();
    void update_state(const Event& event) override;

    void register_event_handlers();

private:
    void handle_motion_detected(const Event& event);
    void handle_motion_cleared(const Event& event);
    void handle_presence_detected(const Event& event);
    void handle_presence_cleared(const Event& event);
};
