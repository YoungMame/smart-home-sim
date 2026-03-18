#pragma once

# include <string>
# include <vector>

// Describes the static characteristics of a device model as stored in SQLite.
// Owned by DeviceEngine; VirtualDevice holds a non-owning const pointer.
struct VirtualDeviceModel {
    std::string              id;
    std::string              label;        // human-readable model title
    std::string              type;
    std::string              protocol;
    std::vector<std::string> capabilities;
    std::vector<std::string> available_events;
};
