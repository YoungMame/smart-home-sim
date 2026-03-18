#include <gtest/gtest.h>

#include "motion_sensor/virtual_motion_sensor.hpp"

namespace {

VirtualDeviceModel make_motion_model() {
    VirtualDeviceModel m;
    m.id = "aqara_motion_p1";
    m.type = "motion";
    m.protocol = "mqtt";
    m.capabilities = {"motion", "presence"};
    m.available_events = {
        "motion.motion_detected",
        "motion.motion_cleared",
        "motion.presence_detected",
        "motion.presence_cleared"
    };
    return m;
}

} // namespace

TEST(VirtualMotionSensorTest, ConstructorStoresMetadata) {
    VirtualDeviceModel m = make_motion_model();
    VirtualMotionSensor sensor("m1", "Motion Hall", "hallway", &m);

    EXPECT_EQ(sensor.id(), "m1");
    EXPECT_EQ(sensor.label(), "Motion Hall");
    EXPECT_EQ(sensor.room(), "hallway");
    EXPECT_EQ(sensor.type(), "motion");
    EXPECT_EQ(sensor.protocol(), "mqtt");
}

TEST(VirtualMotionSensorTest, InitStates_DefaultValues) {
    VirtualDeviceModel m = make_motion_model();
    VirtualMotionSensor sensor("m1", "Motion Hall", "hallway", &m);
    sensor.init_states();

    EXPECT_EQ(sensor.get_state("motion"), "false");
    EXPECT_EQ(sensor.get_state("presence"), "false");
}

TEST(VirtualMotionSensorTest, HasAvailableEvent_MatchesModel) {
    VirtualDeviceModel m = make_motion_model();
    VirtualMotionSensor sensor("m1", "Motion Hall", "hallway", &m);

    EXPECT_TRUE(sensor.has_available_event("motion.motion_detected"));
    EXPECT_TRUE(sensor.has_available_event("motion.presence_cleared"));
    EXPECT_FALSE(sensor.has_available_event("light.turned_on"));
}

TEST(VirtualMotionSensorTest, UpdateState_AppliesPayload) {
    VirtualDeviceModel m = make_motion_model();
    VirtualMotionSensor sensor("m1", "Motion Hall", "hallway", &m);
    sensor.init_states();

    Event e;
    e.type = "state_change";
    e.device_id = "m1";
    e.payload = R"({"motion":"true","presence":"true"})";
    e.scheduled_at = std::chrono::steady_clock::now();

    sensor.update_state(e);

    EXPECT_EQ(sensor.get_state("motion"), "true");
    EXPECT_EQ(sensor.get_state("presence"), "true");
}
