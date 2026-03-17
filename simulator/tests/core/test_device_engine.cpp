#include <gtest/gtest.h>
#include <filesystem>

#include "device_engine/device_engine.hpp"

// DeviceEngine is a singleton — tests share the same instance.
// Each test resets it by reloading from SQLite seed.

static const std::string DB_PATH   = "/tmp/smart_home_sim_test.db";
static const std::string SEED_PATH = "/build/data/seed.sql";

static void reset_db_file() {
    std::error_code ec;
    std::filesystem::remove(DB_PATH, ec);
}

// ── load_from_db ─────────────────────────────────────────────────────────────

TEST(DeviceEngineTest, LoadFromDb_ReturnsCorrectCount) {
    reset_db_file();
    // seed.sql inserts 2 devices (lamp_salon + lamp_chambre).
    int count = DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);
    EXPECT_EQ(count, 2);
}

TEST(DeviceEngineTest, LoadFromDb_DevicesAreRetrievable) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    VirtualDevice* lamp = DeviceEngine::instance().get_device("lamp_salon");
    ASSERT_NE(lamp, nullptr);
    EXPECT_EQ(lamp->id(),    "lamp_salon");
    EXPECT_EQ(lamp->label(), "Lampe du salon");
    EXPECT_EQ(lamp->room(),  "salon");
    EXPECT_EQ(lamp->type(),  "light"); 
}

// ── get_device ────────────────────────────────────────────────────────────────

TEST(DeviceEngineTest, GetDevice_ReturnsNullptrForUnknownId) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    EXPECT_EQ(DeviceEngine::instance().get_device("does_not_exist"), nullptr);
}

TEST(DeviceEngineTest, GetDevice_LampChambreHasColorCapability) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    VirtualDevice* lamp = DeviceEngine::instance().get_device("lamp_chambre");
    ASSERT_NE(lamp, nullptr);
    EXPECT_TRUE(lamp->has_capability("color"));
}

// ── devices() ────────────────────────────────────────────────────────────────

TEST(DeviceEngineTest, DevicesMap_ContainsAllLoadedDevices) {
    reset_db_file();
    DeviceEngine::instance().load_from_db(DB_PATH, SEED_PATH);

    const auto& devs = DeviceEngine::instance().devices();
    EXPECT_GE(devs.size(), 2u);
    EXPECT_NE(devs.find("lamp_salon"),   devs.end());
    EXPECT_NE(devs.find("lamp_chambre"), devs.end());
}
