#include <gtest/gtest.h>
#include "virtual_device_model.hpp"

TEST(VirtualDeviceModelTest, FieldsStoredCorrectly) {
    VirtualDeviceModel m{"ikea_bulb_v1", "IKEA Bulb v1", "light", "mqtt", {"on_off", "brightness"}};

    EXPECT_EQ(m.id,       "ikea_bulb_v1");
    EXPECT_EQ(m.label,    "IKEA Bulb v1");
    EXPECT_EQ(m.type,     "light");
    EXPECT_EQ(m.protocol, "mqtt");
    ASSERT_EQ(m.capabilities.size(), 2);
    EXPECT_EQ(m.capabilities[0], "on_off");
    EXPECT_EQ(m.capabilities[1], "brightness");
}

TEST(VirtualDeviceModelTest, EmptyCapabilities) {
    VirtualDeviceModel m{"bare_device", "Bare Device", "unknown", "rest", {}};
    EXPECT_TRUE(m.capabilities.empty());
}
