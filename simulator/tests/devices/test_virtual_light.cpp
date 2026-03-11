#include <gtest/gtest.h>
#include "light/virtual_light.hpp"

// ── Helpers ───────────────────────────────────────────────────────────────────

static VirtualDeviceModel make_light_model(bool with_color = false) {
    VirtualDeviceModel m;
    m.id       = "ikea_bulb_v1";
    m.type     = "light";
    m.protocol = "mqtt";
    m.capabilities = {"on_off", "brightness"};
    if (with_color)
        m.capabilities.push_back("color");
    return m;
}

// ── Construction ──────────────────────────────────────────────────────────────

TEST(VirtualLightTest, ConstructorStoresMetadata) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);

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

TEST(VirtualLightTest, GetState_ReturnsEmptyForUnknownKey) {
    VirtualDeviceModel m = make_light_model();
    VirtualLight light("l1", "Bureau", "office", &m);

    EXPECT_EQ(light.get_state("nonexistent"), "");
}
