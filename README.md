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
The system architecture is defined in ARCHITECTURE.md, which outlines the modular design of the simulator, including the API layer, core simulation engine, event scheduler, device engine, adapter manager, and supported communication protocols.

## Protocols
The PROTOCOLS.md file describes the communication protocols supported by the simulator, such as MQTT and HTTP, which allow for seamless integration with external applications and devices.

## Technologies Used

### Core
C++: The core simulation engine is implemented in C++ for performance and efficiency.

Fetch library: Used for making HTTP requests to the API.

WebSocket library: Used for real-time communication between the simulator and the dashboard.

### Storage
SQLite: Used for storing device configurations, user data, and simulation state persistently.

At startup, the simulator initializes the SQLite schema and applies a seed script (`simulator/src/data/seed.sql`) to ensure default models and devices exist.

### Infrastructure
Docker: All externally-exposed services run in Docker containers and are accessible to users and external clients.

| Service | Image / Build | Exposed ports |
|---|---|---|
| MQTT Broker | `eclipse-mosquitto:2.1.2-alpine` | `1883` (TCP), `9001` (WS) |
| Virtual Device API | `./api` | `3000` (HTTP REST) |
| WebSocket Server | `./api` | `8080` (WS) |

### Api
Express.js: The Node.js API container acts as a **protocol bridge** between HTTP/WebSocket clients and the MQTT broker. It translates `POST /devices/:type/:id` calls into MQTT commands and streams real-time device state over WebSocket by subscribing to MQTT topics. For historical data and logs it proxies requests to the Simulator API (`core:4000`). Clients that already support MQTT can skip this layer and connect directly to the broker.

### Dashboard
React: The dashboard is built using React for a responsive and interactive user interface.

## Dev environment setup
(Optional) -- Use VS Code dev containers for a seamless development experience with all dependencies pre-configured:
1. Install dev container extension in VS Code
2. Open the project in VS Code and reopen it in the dev container: `Ctrl+Shift+P` → `Remote-Containers: Reopen in Container`
3. When changes are 


3. The dev container will automatically build and start the necessary services. You can also start them manually using Docker Compose (see below).
4. Start the core service to access the CLI: `docker compose --profile broker up core`



## Running with Docker

The simulator ships with a bundled [Eclipse Mosquitto](https://mosquitto.org/)
broker so the whole stack is self-contained. You can also point the services at
an external broker that already exists in your infrastructure.

### Option A — bundled MQTT broker (default)

Start the broker together with the rest of the services using the `broker`
profile:

```bash
docker compose --profile broker up
```

Le service `core` ouvre maintenant un CLI interactif dans le terminal (`smart-home>`).

Stop everything:

```bash
docker compose --profile broker down
```

### Option B — external MQTT broker

If you already have an MQTT broker (HiveMQ, EMQX, Mosquitto on another host,
etc.) you can skip the bundled one. Copy `.env.example` to `.env` and set
`MQTT_BROKER_URL` to your broker's address:

```bash
cp .env.example .env
# edit .env and replace MQTT_BROKER_URL with your broker's address, for example:
# MQTT_BROKER_URL=mqtt://my-broker.local:1883
```

Then start only the core and API services (no `broker` profile):

```bash
docker compose up
```

Optional DB configuration:

```bash
# default values used by the core container
SMART_HOME_DB_PATH=/data/simulator.db
SMART_HOME_DB_SEED=/data/seed.sql
```

### Running unit tests

```bash
docker compose --profile test run --build --rm test
```

### Exposed endpoints

| Protocol | URL | Description |
|---|---|---|
| MQTT | `mqtt://localhost:1883` | MQTT broker (TCP) |
| MQTT over WS | `ws://localhost:9001` | MQTT broker (WebSocket) |
| HTTP REST | `http://localhost:3000` | Virtual device API |
| WebSocket | `ws://localhost:8080` | Real-time device events |


