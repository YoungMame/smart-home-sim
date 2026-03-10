cmake_minimum_required(VERSION 3.20)
project(smart_home_sim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build type default ────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Dependencies (fetched at configure time) ──────────────────────────────────
include(FetchContent)

# nlohmann/json — JSON parsing for device_models.json / devices.json
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

FetchContent_MakeAvailable(nlohmann_json httplib)

# ── System libraries ──────────────────────────────────────────────────────────
find_package(Threads REQUIRED)

# Eclipse Mosquitto C client (apt: libmosquitto-dev)
find_library(MOSQUITTO_LIB mosquitto REQUIRED)
find_path(MOSQUITTO_INCLUDE mosquitto.h REQUIRED)

# ── Sources ───────────────────────────────────────────────────────────────────
# Add source files here as they are created in each sub-module.
# Using explicit lists (preferred over GLOB) so CMake reruns on new files.
set(CORE_SOURCES
  core/scheduler/event_scheduler.cpp
  core/device_engine/device_engine.cpp
  # core/event_engine/event_engine.cpp
)

set(DEVICE_SOURCES
  devices/virtual_device.cpp
  devices/light/virtual_light.cpp
  devices/thermostat/virtual_thermostat.cpp
  # devices/motion_sensor/motion_sensor.cpp
)

set(PROTOCOL_SOURCES
  # protocols/mqtt/mqtt_adapter.cpp
  # protocols/rest/rest_adapter.cpp
  # protocols/ws/ws_adapter.cpp
)

add_executable(smart-home-sim
  main.cpp
  ${CORE_SOURCES}
  ${DEVICE_SOURCES}
  ${PROTOCOL_SOURCES}
)

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(smart-home-sim PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/core
  ${CMAKE_CURRENT_SOURCE_DIR}/devices
  ${CMAKE_CURRENT_SOURCE_DIR}/protocols
  ${MOSQUITTO_INCLUDE}
)

# ── Link libraries ────────────────────────────────────────────────────────────
target_link_libraries(smart-home-sim PRIVATE
  Threads::Threads
  ${MOSQUITTO_LIB}
  nlohmann_json::nlohmann_json
  httplib::httplib
)

# ── Compile options ───────────────────────────────────────────────────────────
target_compile_options(smart-home-sim PRIVATE
  $<$<CONFIG:Release>:-O2>
  $<$<CONFIG:Debug>:-g -fsanitize=address>
)
target_link_options(smart-home-sim PRIVATE
  $<$<CONFIG:Debug>:-fsanitize=address>
)

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS smart-home-sim DESTINATION bin)
install(DIRECTORY data/ DESTINATION share/smart-home-sim/data)
