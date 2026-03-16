cmake_minimum_required(VERSION 3.20)

# ── Google Test ───────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
)
# Do not install gtest alongside the project binaries.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Test executable ───────────────────────────────────────────────────────────
add_executable(smart-home-tests
  devices/test_virtual_device_model.cpp
  devices/test_virtual_light.cpp
  devices/test_virtual_thermostat.cpp
  core/test_device_engine.cpp
  protocols/test_virtual_device_mqtt.cpp
  protocols/test_rest_components.cpp
)

target_link_libraries(smart-home-tests PRIVATE
  sim_core
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(smart-home-tests)
