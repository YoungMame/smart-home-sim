#include <filesystem>

#include <gtest/gtest.h>

#include "device_engine/device_engine.hpp"
#include "event_engine/event_engine.hpp"

namespace {

static const std::string DB_PATH = "/tmp/smart_home_sim_test.db";
static const std::string SEED_PATH = "/build/data/seed.sql";

void reset_db_file() {
    std::error_code ec;
    std::filesystem::remove(DB_PATH, ec);
}

Event make_event(const std::string& type, const std::string& device_id, const std::string& payload) {
    Event e;
    e.type = type;
    e.device_id = device_id;
    e.payload = payload;
    e.scheduled_at = std::chrono::steady_clock::now();
    return e;
}

} // namespace

TEST(EventEngineTest, ProcessEvent_LightTurnedOn_UpdatesState) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    EventEngine::instance().process_event(make_event("light.turned_on", "lamp_salon", "{}"));

    auto* light = DeviceEngine::instance().get_device("lamp_salon");
    ASSERT_NE(light, nullptr);
    EXPECT_EQ(light->get_state("on"), "true");
}

TEST(EventEngineTest, ProcessEvent_LightBrightnessChanged_UpdatesState) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    EventEngine::instance().process_event(make_event("light.brightness_changed", "lamp_salon", R"({"brightness":"42"})"));

    auto* light = DeviceEngine::instance().get_device("lamp_salon");
    ASSERT_NE(light, nullptr);
    EXPECT_EQ(light->get_state("brightness"), "42");
}

TEST(EventEngineTest, ProcessEvent_ThermostatSetpointChanged_UpdatesState) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    ASSERT_TRUE(DeviceEngine::instance().add_device("thermo_test", "Thermo Test", "office", "nest_thermostat"));

    EventEngine::instance().process_event(make_event("thermostat.setpoint_changed", "thermo_test", R"({"target_temperature":"19.5"})"));

    auto* thermostat = DeviceEngine::instance().get_device("thermo_test");
    ASSERT_NE(thermostat, nullptr);
    EXPECT_EQ(thermostat->get_state("target_temperature"), "19.5");
}

TEST(EventEngineTest, ProcessEvent_UnknownPrefixedEvent_DoesNotChangeState) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    auto* light = DeviceEngine::instance().get_device("lamp_salon");
    ASSERT_NE(light, nullptr);
    const std::string before = light->get_state("on");

    EventEngine::instance().process_event(make_event("light.not_existing", "lamp_salon", "{}"));

    EXPECT_EQ(light->get_state("on"), before);
}

TEST(EventEngineTest, ProcessEvent_MotionDetected_UpdatesState) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    ASSERT_TRUE(DeviceEngine::instance().add_device("motion_test", "Motion Test", "office", "aqara_motion_p1"));

    EventEngine::instance().process_event(make_event("motion.motion_detected", "motion_test", "{}"));

    auto* sensor = DeviceEngine::instance().get_device("motion_test");
    ASSERT_NE(sensor, nullptr);
    EXPECT_EQ(sensor->get_state("motion"), "true");
}
