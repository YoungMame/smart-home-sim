#include <gtest/gtest.h>
#include "thermostat/virtual_thermostat.hpp"

// ── Helpers ───────────────────────────────────────────────────────────────────

static VirtualDeviceModel make_thermostat_model() {
    VirtualDeviceModel m;
    m.id           = "nest_thermostat";
    m.type         = "thermostat";
    m.protocol     = "ws";
    m.capabilities = {"temperature", "target_temperature"};
    return m;
}

// ── Construction ──────────────────────────────────────────────────────────────

TEST(VirtualThermostatTest, ConstructorStoresMetadata) {
    VirtualDeviceModel m = make_thermostat_model();
    VirtualThermostat t("t1", "Thermostat Salon", "living_room", &m);

    EXPECT_EQ(t.id(),       "t1");
    EXPECT_EQ(t.label(),    "Thermostat Salon");
    EXPECT_EQ(t.room(),     "living_room");
    EXPECT_EQ(t.type(),     "thermostat");
    EXPECT_EQ(t.protocol(), "ws");
}

// ── init_states ───────────────────────────────────────────────────────────────

TEST(VirtualThermostatTest, InitStates_DefaultValues) {
    VirtualDeviceModel m = make_thermostat_model();
    VirtualThermostat t("t1", "Thermostat Salon", "living_room", &m);
    t.init_states();

    EXPECT_EQ(t.get_state("temperature"),        "20.0");
    EXPECT_EQ(t.get_state("target_temperature"), "21.0");
}

// ── has_capability ────────────────────────────────────────────────────────────

TEST(VirtualThermostatTest, HasCapability_Temperature) {
    VirtualDeviceModel m = make_thermostat_model();
    VirtualThermostat t("t1", "Thermostat Salon", "living_room", &m);

    EXPECT_TRUE(t.has_capability("temperature"));
    EXPECT_TRUE(t.has_capability("target_temperature"));
    EXPECT_FALSE(t.has_capability("on_off"));
}

// ── set_state / get_state ─────────────────────────────────────────────────────

TEST(VirtualThermostatTest, SetState_UpdatesTemperature) {
    VirtualDeviceModel m = make_thermostat_model();
    VirtualThermostat t("t1", "Thermostat Salon", "living_room", &m);
    t.init_states();

    t.set_state("temperature", "22.5");
    EXPECT_EQ(t.get_state("temperature"), "22.5");
}

TEST(VirtualThermostatTest, SetState_UpdatesTargetTemperature) {
    VirtualDeviceModel m = make_thermostat_model();
    VirtualThermostat t("t1", "Thermostat Salon", "living_room", &m);
    t.init_states();

    t.set_state("target_temperature", "19.0");
    EXPECT_EQ(t.get_state("target_temperature"), "19.0");
}

TEST(VirtualThermostatTest, GetState_ReturnsEmptyForUnknownKey) {
    VirtualDeviceModel m = make_thermostat_model();
    VirtualThermostat t("t1", "Thermostat Salon", "living_room", &m);

    EXPECT_EQ(t.get_state("color"), "");
}
