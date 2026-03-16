#pragma once

#include <map>
#include <string>
#include <vector>

#include "core/adapter_manager/protocol_client.hpp"
#include "event_engine/event.hpp"
#include "virtual_device_model.hpp"

// Abstract base class for all virtual smart devices.
// Each device subtype (light, thermostat, …) must implement update_state().
class VirtualDevice {
public:
    VirtualDevice(std::string id,
                  std::string label,
                  std::string room,
                  const VirtualDeviceModel* model);

    virtual ~VirtualDevice() = default;

    // Non-copyable (devices are owned by DeviceEngine via unique_ptr).
    VirtualDevice(const VirtualDevice&)            = delete;
    VirtualDevice& operator=(const VirtualDevice&) = delete;
    VirtualDevice(VirtualDevice&&)                 = default;
    VirtualDevice& operator=(VirtualDevice&&)      = default;

    const std::string&        id()       const { return id_; }
    const std::string&        label()    const { return label_; }
    const std::string&        room()     const { return room_; }
    const VirtualDeviceModel* model()    const { return model_; }
    const std::string&        type()     const { return model_->type; }
    const std::string&        protocol() const { return model_->protocol; }
    const std::vector<std::string>& capabilities() const { return model_->capabilities; }
    const std::string&        modelId()   const { return model_->id; }
    const VirtualDeviceModel* model_ptr() const { return model_; }
    bool has_capability(const std::string& cap) const;

    // Returns empty string if key is absent.
    std::string get_state(const std::string& key) const;
    const std::map<std::string, std::string>& states() const { return states_; }
    void        set_state(const std::string& key, const std::string& value);

    // MQTT — non-owning pointer, set by DeviceEngine after construction.
    // Only relevant when protocol() == "mqtt".
    void set_mqtt_client(ProtocolClient* client) { mqtt_client_ = client; }
    ProtocolClient* mqtt_client() const { return mqtt_client_; }

    // Publishes the full current state as JSON to home/<type>/<id>/state.
    // No-op if no MQTT client is set.
    void publish_state() const;

    // Returns the MQTT topic used by this device for state publication.
    std::string state_topic() const;

    // Called by DeviceEngine when an event targets this device.
    virtual void update_state(const Event& event) = 0;

protected:
    void apply_state_payload(const std::string& payload);

    std::string                        id_;
    std::string                        label_;
    std::string                        room_;
    const VirtualDeviceModel*          model_;
    std::map<std::string, std::string> states_;

private:
    ProtocolClient* mqtt_client_{ nullptr };
};
