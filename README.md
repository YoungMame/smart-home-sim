# Smart Home Simulator

## Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) — System architecture diagram and component overview
- [DATA_MODEL.md](DATA_MODEL.md) — Classes and data structures that compose the project
- [CODING_STYLE.md](CODING_STYLE.md) — Code formatting and style conventions
- [CONTRIBUTING.md](CONTRIBUTING.md) — Guidelines for contributing to the project

---

Smart Home Simulator is a software application that simulates a smart home environment. It allows users to create and manage virtual smart devices, such as lights, thermostats, and security cameras, and control them through a user-friendly interface. The simulator provides a realistic experience of how smart home technology works and can be used for testing and educational purposes.

![alt text](https://github.com/YoungMame/smart-home-sim/blob/main/media/functionning.png?raw=true)

## System Architecture
The system arhcitecure is defined in ARCHITECTURE.md, which outlines the modular design of the simulator, including the API layer, core simulation engine, event scheduler, device engine, adapter manager, and supported communication protocols.

## Protocols
The PROTOCOLS.md file describes the communication protocols supported by the simulator, such as MQTT and HTTP, which allow for seamless integration with external applications and devices.

## Technologies Used

### Core
C++: The core simulation engine is implemented in C++ for performance and efficiency.

Fetch library: Used for making HTTP requests to the API.

WebSocket library: Used for real-time communication between the simulator and the dashboard.

### Storage
SQLite: Used for storing device configurations, user data, and simulation state persistently.

### Infrastructure
Docker: All externally-exposed services run in Docker containers and are accessible to users and external clients.

| Service | Image / Build | Exposed ports |
|---|---|---|
| MQTT Broker | `eclipse-mosquitto:2.0` | `1883` (TCP), `9001` (WS) |
| Virtual Device API | `./api` | `3000` (HTTP REST) |
| WebSocket Server | `./api` | `8080` (WS) |

### Api
Express.js: The API is built using Express.js, a web application framework for Node.js, to handle HTTP requests and responses.

### Dashboard
React: The dashboard is built using React for a responsive and interactive user interface.

## Running with Docker

Start all services with:

```bash
docker compose up -d
```

Stop all services:

```bash
docker compose down
```

### Exposed endpoints

| Protocol | URL | Description |
|---|---|---|
| MQTT | `mqtt://localhost:1883` | MQTT broker (TCP) |
| MQTT over WS | `ws://localhost:9001` | MQTT broker (WebSocket) |
| HTTP REST | `http://localhost:3000` | Virtual device API |
| WebSocket | `ws://localhost:8080` | Real-time device events |


