graph TD

    API[API Layer]

    CORE[Simulator Core]

    SCHED[Event Scheduler<br>Main Loop]
    EVENT_ENGINE[Event Engine]
    DEVICE_ENGINE[Device Engine]
    DEVICES[Virtual Devices]

    ADAPTER_MANAGER[Adapter Manager]
    MQTT_ADAPTER[MQTT Adapter]
    REST_ADAPTER[REST Adapter]
    WS_ADAPTER[WebSocket Adapter]

    MQTT_BROKER[MQTT Broker]

    DEVICE_CONST[Device Definitions<br>(supported devices)]

    DB[(SQLite Database)]

    API -->|trigger actions / fetch data| CORE

    CORE --> SCHED
    SCHED --> EVENT_ENGINE
    EVENT_ENGINE --> DEVICE_ENGINE
    DEVICE_ENGINE --> DEVICES

    CORE --> ADAPTER_MANAGER

    ADAPTER_MANAGER --> MQTT_ADAPTER
    ADAPTER_MANAGER --> REST_ADAPTER
    ADAPTER_MANAGER --> WS_ADAPTER

    MQTT_ADAPTER --> MQTT_BROKER

    DEVICE_ENGINE --> DEVICE_CONST

    CORE --> DB
    API --> DB