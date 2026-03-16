This document lists the dependencies used in the Smart Home Simulator project, including build-time, runtime, and test dependencies.

## Core Simulation Engine (C++)

The simulator core is built with CMake and C++17.

### Toolchain Requirements

| Dependency | Version | Purpose |
|---|---|---|
| C++ compiler with C++17 support | C++17+ | Builds the simulator core and tests. |
| CMake | 3.20+ | Configures and generates build files. |
| Ninja (recommended) | latest | Fast generator used by Docker/dev workflow. |
| Git | latest | Fetches CMake `FetchContent` dependencies. |
| POSIX shell (`sh`) | latest | Runs `simulator/watch.sh` in the dev container. |

### System Libraries (native)

| Dependency | Version | Purpose |
|---|---|---|
| SQLite3 | system package | Persistent storage for simulator data/state. |
| Eclipse Mosquitto C client (`libmosquitto`) | system package | MQTT connectivity from C++ core. |
| OpenSSL | system package | TLS/runtime crypto dependency for Mosquitto stack. |
| Threads | system package | Multithreading support (`find_package(Threads)`). |

### CMake Fetched Dependencies

These are downloaded automatically during CMake configure.

| Dependency | Version | Header-only (IDE) | Purpose |
|---|---|---|---|
| nlohmann/json | v3.11.3 | Yes | JSON parsing and serialization. |
| cpp-httplib | v0.18.3 | Yes | Internal HTTP API server utilities. |
| argparse | v3.2 | Yes | Command-line argument parsing. |

`Header-only (IDE) = Yes` means you only need headers for editor IntelliSense/autocomplete. No separate compiled system library install is required.

### Test Dependency (C++)

| Dependency | Version | Header-only (IDE) | Purpose |
|---|---|---|---|
| GoogleTest | v1.15.2 | No | Unit testing framework (fetched in `simulator/tests/CMakeLists.txt`). |

## API Service (Node.js)

The `api` service exposes REST and WebSocket interfaces and bridges to MQTT.

### Runtime

| Dependency | Version | Purpose |
|---|---|---|
| Node.js | 20 (Docker image) | Runs the API process. |
| npm | bundled with Node 20 | Installs JavaScript dependencies. |

### NPM Dependencies

| Package | Version (package.json) | Purpose |
|---|---|---|
| express | ^4.18.2 | HTTP REST routes for devices and data access. |
| mqtt | ^5.3.4 | MQTT publish/subscribe bridge. |
| ws | ^8.16.0 | WebSocket server for real-time updates. |

## Infrastructure / Containers

| Component | Version | Purpose |
|---|---|---|
| Docker Engine | recent stable | Runs all services in containers. |
| Docker Compose (v2) | recent stable | Orchestrates `core`, `api`, `mqtt-broker`, and test profiles. |
| Eclipse Mosquitto image | 2.1.2-alpine | Bundled MQTT broker (`mqtt-broker` service). |
| Alpine Linux base image | 3.21 | Base image for simulator build/runtime/dev/test containers. |
| Node base image | node:20-alpine | Base image for API container. |

## Optional / External Services

| Dependency | Version | Purpose |
|---|---|---|
| External MQTT broker (HiveMQ, EMQX, Mosquitto, etc.) | any MQTT-compatible | Optional alternative to bundled broker via `MQTT_BROKER_URL`. |

## Environment Variables Related to Dependencies

| Variable | Default | Purpose |
|---|---|---|
| `MQTT_BROKER_URL` | `mqtt://mqtt-broker:1883` | MQTT endpoint used by `core` and `api`. |
| `CORE_PORT` | `4000` | Internal simulator API port. |
| `SMART_HOME_DB_PATH` | `/data/simulator.db` (container default) | SQLite database file path. |
| `SMART_HOME_DB_SEED` | `/data/seed.sql` (container default) | Seed SQL file path used on startup. |

## Installation Notes

- Recommended path: run with Docker Compose so dependencies are provisioned automatically.
- Native local builds require manually installing C++ toolchain, CMake, SQLite3 dev libs, Mosquitto dev libs, and OpenSSL dev libs.
- Test builds require internet access at configure time to fetch GoogleTest and other CMake `FetchContent` dependencies.

## Install system libraries

### Ubuntu/Debian
```bash
sudo apt update
sudo apt install libsqlite3-dev libmosquitto-dev libssl-dev
```

### Windows (using WSL2)
```bash
sudo apt update
sudo apt install libsqlite3-dev libmosquitto-dev libssl-dev
```

### Windows (using MSYS2 mingw64-ucrt)
```bash
pacman -S mingw-w64-ucrt-x86_64-sqlite3 mingw-w64-ucrt-x86_64-mosquitto mingw-w64-ucrt-x86_64-openssl
```

### Windows (using MSYS2 mingw64)
```bash
pacman -S mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-mosquitto mingw-w64-x86_64-openssl
```

### Windows (using MSYS2 msys2)
```bash
pacman -S sqlite3 mosquitto openssl
```


