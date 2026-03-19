#pragma once

# include <map>
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
    std::map<std::string, std::string> capability_aliases; // alias -> canonical capability
    std::vector<std::string> available_events;
};
