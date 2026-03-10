```mermaid
graph TD

    USER([User / External Client])

    CORE[Simulator Core]

    SCHED[Event Scheduler<br>Main Loop]
    EVENT_ENGINE[Event Engine]
    DEVICE_ENGINE[Device Engine]
    DEVICES[Virtual Devices]

    ADAPTER_MANAGER[Adapter Manager]
    MQTT_ADAPTER[MQTT Adapter]
    REST_ADAPTER[REST Adapter]
    WS_ADAPTER[WebSocket Adapter]

    DEVICE_CONST[Device Definitions(supported devices)]

    DB[(SQLite Database)]

    subgraph DOCKER["Docker"]
        MQTT_BROKER["MQTT Broker<br>:1883 / :9001"]
        API["Virtual Device API<br>:3000"]
        WS_SERVER["WebSocket Server<br>:8080"]
    end

    USER -->|"MQTT :1883"| MQTT_BROKER
    USER -->|"HTTP :3000"| API
    USER -->|"WS :8080"| WS_SERVER

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
    REST_ADAPTER --> API
    WS_ADAPTER --> WS_SERVER

    DEVICE_ENGINE --> DEVICE_CONST

    CORE --> DB
    API --> DB

```
