#include <gtest/gtest.h>
#include "device_engine/device_engine.hpp"

// DeviceEngine is a singleton — tests share the same instance.
// Each fixture resets it by loading fresh JSON paths.

static const std::string MODELS_PATH  = "/build/data/device_models.json";
static const std::string DEVICES_PATH = "/build/data/devices.json";

// ── load_from_json ────────────────────────────────────────────────────────────

TEST(DeviceEngineTest, LoadFromJson_ReturnsCorrectCount) {
    // devices.json contains 2 devices (lamp_salon + lamp_chambre).
    int count = DeviceEngine::instance().load_from_json(MODELS_PATH, DEVICES_PATH);
    EXPECT_EQ(count, 2);
}

TEST(DeviceEngineTest, LoadFromJson_DevicesAreRetrievable) {
    DeviceEngine::instance().load_from_json(MODELS_PATH, DEVICES_PATH);

    VirtualDevice* lamp = DeviceEngine::instance().get_device("lamp_salon");
    ASSERT_NE(lamp, nullptr);
    EXPECT_EQ(lamp->id(),    "lamp_salon");
    EXPECT_EQ(lamp->label(), "Lampe du salon");
    EXPECT_EQ(lamp->room(),  "salon");
    EXPECT_EQ(lamp->type(),  "light");
}

// ── get_device ────────────────────────────────────────────────────────────────

TEST(DeviceEngineTest, GetDevice_ReturnsNullptrForUnknownId) {
    DeviceEngine::instance().load_from_json(MODELS_PATH, DEVICES_PATH);

    EXPECT_EQ(DeviceEngine::instance().get_device("does_not_exist"), nullptr);
}

TEST(DeviceEngineTest, GetDevice_LampChambreHasColorCapability) {
    DeviceEngine::instance().load_from_json(MODELS_PATH, DEVICES_PATH);

    VirtualDevice* lamp = DeviceEngine::instance().get_device("lamp_chambre");
    ASSERT_NE(lamp, nullptr);
    EXPECT_TRUE(lamp->has_capability("color"));
}

// ── devices() ────────────────────────────────────────────────────────────────

TEST(DeviceEngineTest, DevicesMap_ContainsAllLoadedDevices) {
    DeviceEngine::instance().load_from_json(MODELS_PATH, DEVICES_PATH);

    const auto& devs = DeviceEngine::instance().devices();
    EXPECT_GE(devs.size(), 2u);
    EXPECT_NE(devs.find("lamp_salon"),   devs.end());
    EXPECT_NE(devs.find("lamp_chambre"), devs.end());
}
