#include <gtest/gtest.h>
#include <chrono>
#include "light/virtual_light.hpp"

// ── Helpers ───────────────────────────────────────────────────────────────────

static VirtualDeviceModel make_light_model(bool with_color = false) {
    VirtualDeviceModel m;
    m.id       = "ikea_bulb_v1";
    m.type     = "light";
    m.protocol = "mqtt";
    m.capabilities = {"on_off", "brightness"};
    m.capability_aliases = {
        {"power", "on_off"},
        {"state", "on_off"},
        {"dimmer", "brightness"}
    };
    m.available_events = {"light.turned_on", "light.turned_off", "light.brightness_changed"};
    if (with_color)
        m.capabilities.push_back("color");
    if (with_color)
        m.capability_aliases["hue"] = "color";
    if (with_color)
        m.available_events.push_back("light.color_changed");
    return m;
}

// ── Construction ──────────────────────────────────────────────────────────────

TEST(VirtualLightTest, ConstructorStoresMetadata) {
    VirtualDeviceModel m = make_light_model();
    std::cout << "model made" << std::endl;
    VirtualLight light("l1", "Bureau", "office", &m);
    std::cout << "light constructed" << std::endl;

    EXPECT_EQ(light.id(),       "l1");
    EXPECT_EQ(light.label(),    "Bureau");
    EXPECT_EQ(light.room(),     "office");
    EXPECT_EQ(light.type(),     "light");
    EXPECT_EQ(light.protocol(), "mqtt");
}

// ── init_states ───────────────────────────────────────────────────────────────

TEST(VirtualLightTest, InitStates_DefaultValues) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);
    light.init_states();

    EXPECT_EQ(light.get_state("on"),         "false");
    EXPECT_EQ(light.get_state("brightness"), "100");
}

TEST(VirtualLightTest, InitStates_WithColorCapability) {
    VirtualDeviceModel m = make_light_model(/*with_color=*/true);
    VirtualLight light("l2", "Salon", "living_room", &m);
    light.init_states();

    EXPECT_EQ(light.get_state("color"), "#ffffff");
}

TEST(VirtualLightTest, InitStates_NoColorState_WhenCapabilityAbsent) {
    VirtualDeviceModel m = make_light_model(/*with_color=*/false);
    VirtualLight light("l3", "Couloir", "hallway", &m);
    light.init_states();

    // get_state returns "" for absent keys.
    EXPECT_EQ(light.get_state("color"), "");
}

// ── has_capability ────────────────────────────────────────────────────────────

TEST(VirtualLightTest, HasCapability_ReturnsTrueForPresentCap) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);

    EXPECT_TRUE(light.has_capability("on_off"));
    EXPECT_TRUE(light.has_capability("brightness"));
}

TEST(VirtualLightTest, HasCapability_ReturnsFalseForAbsentCap) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);

    EXPECT_FALSE(light.has_capability("color"));
    EXPECT_FALSE(light.has_capability("temperature"));
}

TEST(VirtualLightTest, HasAvailableEvent_ReturnsTrueForPresentEvent) {
    VirtualDeviceModel m = make_light_model(/*with_color=*/true);
    VirtualLight light("l1", "Bureau", "office", &m);

    EXPECT_TRUE(light.has_available_event("light.turned_on"));
    EXPECT_TRUE(light.has_available_event("light.color_changed"));
}

TEST(VirtualLightTest, HasAvailableEvent_ReturnsFalseForAbsentEvent) {
    VirtualDeviceModel m = make_light_model(/*with_color=*/false);
    VirtualLight light("l1", "Bureau", "office", &m);

    EXPECT_FALSE(light.has_available_event("light.color_changed"));
    EXPECT_FALSE(light.has_available_event("thermostat.mode_changed"));
}

// ── set_state / get_state ─────────────────────────────────────────────────────

TEST(VirtualLightTest, SetState_OverwritesValue) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);
    light.init_states();

    light.set_state("on", "true");
    EXPECT_EQ(light.get_state("on"), "true");

    light.set_state("brightness", "50");
    EXPECT_EQ(light.get_state("brightness"), "50");
}

TEST(VirtualLightTest, UpdateState_ResolvesAliasToCanonicalStateKey) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);
    light.init_states();

    Event e;
    e.type = "state_change";
    e.device_id = "l1";
    e.payload = R"({"power":"true","dimmer":"45"})";
    e.scheduled_at = std::chrono::steady_clock::now();

    light.update_state(e);

    EXPECT_EQ(light.get_state("on"), "true");
    EXPECT_EQ(light.get_state("brightness"), "45");
}

TEST(VirtualLightTest, UpdateState_IgnoresUnknownVariables) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);
    light.init_states();

    Event e;
    e.type = "state_change";
    e.device_id = "l1";
    e.payload = R"({"unknown_variable":"abc"})";
    e.scheduled_at = std::chrono::steady_clock::now();

    light.update_state(e);

    EXPECT_EQ(light.get_state("unknown_variable"), "");
}

TEST(VirtualLightTest, GetState_ReturnsEmptyForUnknownKey) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);

    EXPECT_EQ(light.get_state("nonexistent"), "");
}
