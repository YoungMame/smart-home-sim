#include "virtual_device.hpp"

#include <algorithm>

VirtualDevice::VirtualDevice(std::string id,
                              std::string label,
                              std::string room,
                              const VirtualDeviceModel* model)
    : id_(std::move(id))
    , label_(std::move(label))
    , room_(std::move(room))
    , model_(model)
{}

bool VirtualDevice::has_capability(const std::string& cap) const {
    const auto& caps = model_->capabilities;
    return std::find(caps.begin(), caps.end(), cap) != caps.end();
}

std::string VirtualDevice::get_state(const std::string& key) const {
    auto it = states_.find(key);
    return it != states_.end() ? it->second : "";
}

void VirtualDevice::set_state(const std::string& key, const std::string& value) {
    states_[key] = value;
}
