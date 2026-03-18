# Data Model

This document describes the main classes that compose the Smart Home Simulator.

---

## EventScheduler

Manages the scheduling of events in the simulation.

```cpp
class EventScheduler {
    void schedule_event(Event event);
    void run();
};
```

## EventEngine

Processes events and updates the state of devices accordingly.

```cpp
class EventEngine {
    void process_event(Event event);
};
```

## DeviceEngine

Handles the logic for updating device states based on events and interactions.

```cpp
class DeviceEngine {
    void update_device_state(Device device, Event event);
};
```

## VirtualDevice

Represents a virtual smart device in the simulation.

```cpp
class VirtualDevice {
    string              name;
    string              type;
    map<string, string> states;

    void update_state();
};
```

## AdapterManager

Manages the communication adapters for different protocols (MQTT, REST, WebSocket).

```cpp
class AdapterManager {
    void register_adapter(ProtocolAdapter adapter);
    void send_message(string protocol, string topic, string message);
};
```

## ProtocolAdapter

Interface for communication adapters. Specific adapters for MQTT, REST, and WebSocket inherit from this class.

```cpp
class ProtocolAdapter {
    virtual void send_message(string topic, string message) = 0;
};
```
