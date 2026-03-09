## EventScheduler
- Manages the scheduling of events in the simulation
- run() method starts the event loop and processes events in a timely manner

{
    void schedule_event(Event event);
    void run();
};

## EventEngine
- Processes events and updates the state of devices accordingly

{
    void process_event(Event event);
};

## DeviceEngine
- Manages the virtual devices in the simulation
- update_device_state() method updates the state of a device based on events

{
    void update_device_state(Device device, Event event);
};

## VirtualDevice
- Represents a virtual smart device in the simulation

{
    string              name;
    string              type;
    map<string, string> states;

    void update_state();
};

## AdapterManager
- Manages the communication adapters for different protocols (MQTT, REST, WebSocket)

{
    void register_adapter(ProtocolAdapter adapter);
    void send_message(string protocol, string topic, string message);
};

## ProtocolAdapter
- Interface for communication adapters
- Allow to make inherit specific adapters for MQTT, REST, WebSocket, etc.

{
    virtual void send_message(string topic, string message) = 0;
};