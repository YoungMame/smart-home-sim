#pragma once

#include <string>
#include <vector>

// Describes the static characteristics of a device model as defined in device_models.json.
// Owned by DeviceEngine; VirtualDevice holds a non-owning const pointer.
struct VirtualDeviceModel {
    std::string              id;
    std::string              label;        // human-readable model title
    std::string              type;
    std::string              protocol;
    std::vector<std::string> capabilities;
};
