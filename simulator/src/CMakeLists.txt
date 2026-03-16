cmake_minimum_required(VERSION 3.20)
project(smart_home_sim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Build type default ────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Dependencies (fetched at configure time) ──────────────────────────────────
include(FetchContent)

# nlohmann/json — JSON parsing for event payloads and API serialization
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE
)

# cpp-httplib — header-only HTTP/WebSocket server (internal API on port 4000)
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.18.3
  GIT_SHALLOW    TRUE
)

# Disable install/package logic for fetched dependencies.
set(HTTPLIB_INSTALL OFF CACHE BOOL "Disable cpp-httplib install targets" FORCE)
set(HTTPLIB_TEST OFF CACHE BOOL "Disable cpp-httplib tests" FORCE)

# argparse — command-line argument parsing
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG        v3.2
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(nlohmann_json httplib argparse)

# ── System libraries ──────────────────────────────────────────────────────────
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

# Eclipse Mosquitto C client (apt: libmosquitto-dev)
find_library(MOSQUITTO_LIB mosquitto REQUIRED)
find_path(MOSQUITTO_INCLUDE mosquitto.h REQUIRED)

# ── Sources ───────────────────────────────────────────────────────────────────
# Add source files here as they are created in each sub-module.
# Using explicit lists (preferred over GLOB) so CMake reruns on new files.
set(CORE_SOURCES
  core/adapter_manager/adapter_manager.cpp
  core/adapter_manager/protocol_client.cpp
  core/scheduler/event_scheduler.cpp
  core/device_engine/device_engine.cpp
  core/event_engine/event_engine.cpp
  core/db/sqlite_store.cpp
)

set(DEVICE_SOURCES
  devices/virtual_device.cpp
  devices/light/virtual_light.cpp
  devices/thermostat/virtual_thermostat.cpp
  # devices/motion_sensor/motion_sensor.cpp
)

set(PROTOCOL_SOURCES
  protocols/mqtt/mqtt_client.cpp
  protocols/rest/rest_client.cpp
  protocols/rest/rest_server.cpp
  protocols/rest/rest_servers_manager.cpp
  protocols/ws/ws_client.cpp
  protocols/ws/ws_servers_manager.cpp
  # protocols/mqtt/mqtt_adapter.cpp
)

set(SERVER_SOURCES
  server/simulator_api.cpp
)

# ── sim_core static library (shared by the main binary and tests) ─────────────
add_library(sim_core STATIC
  ${CORE_SOURCES}
  ${DEVICE_SOURCES}
  ${PROTOCOL_SOURCES}
  ${SERVER_SOURCES}
)

target_include_directories(sim_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/core
  ${CMAKE_CURRENT_SOURCE_DIR}/devices
  ${CMAKE_CURRENT_SOURCE_DIR}/protocols
  ${MOSQUITTO_INCLUDE}
)

target_link_libraries(sim_core PUBLIC
  Threads::Threads
  SQLite::SQLite3
  ${MOSQUITTO_LIB}
  nlohmann_json::nlohmann_json
  httplib::httplib
  argparse::argparse
)

target_compile_options(sim_core PRIVATE
  $<$<CONFIG:Release>:-O2>
  $<$<CONFIG:Debug>:-g>
)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(smart-home-sim
  main.cpp
  cli/cli.cpp
)
target_link_libraries(smart-home-sim PRIVATE sim_core)

# ── Compile options ───────────────────────────────────────────────────────────
target_compile_options(smart-home-sim PRIVATE
  $<$<CONFIG:Release>:-O2>
  $<$<CONFIG:Debug>:-g>
)

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS smart-home-sim DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/smart-home-sim/data)

# ── Tests ─────────────────────────────────────────────────────────────────────
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(../tests tests_build)
endif()
