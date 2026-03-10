#include "virtual_device.hpp"

#include <algorithm>

VirtualDevice::VirtualDevice(std::string id,
                              std::string label,
                              std::string room,
                              std::string model,
                              std::string protocol,
                              std::vector<std::string> capabilities)
    : id_(std::move(id))
    , label_(std::move(label))
    , room_(std::move(room))
    , model_(std::move(model))
    , protocol_(std::move(protocol))
    , capabilities_(std::move(capabilities))
{}

bool VirtualDevice::has_capability(const std::string& cap) const {
    return std::find(capabilities_.begin(), capabilities_.end(), cap) != capabilities_.end();
}

std::string VirtualDevice::get_state(const std::string& key) const {
    auto it = states_.find(key);
    return it != states_.end() ? it->second : "";
}

void VirtualDevice::set_state(const std::string& key, const std::string& value) {
    states_[key] = value;
}
