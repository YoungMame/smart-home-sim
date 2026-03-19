#include <gtest/gtest.h>

#include "light/virtual_light.hpp"
#include "mqtt/mqtt_client.hpp"

// ── Mock ──────────────────────────────────────────────────────────────────────

struct MockMQTTClient : ProtocolClient {
    std::string last_topic;
    std::string last_message;
    int         publish_call_count{ 0 };

    AdapterProtocol protocol() const override { return AdapterProtocol::Mqtt; }

    void connect(const std::string&) override {}
    void disconnect() override {}
    bool is_connected() const override { return true; }

    void send(const std::string& topic, const std::string& message) override {
        last_topic   = topic;
        last_message = message;
        ++publish_call_count;
    }

    std::vector<SimulatedMessage> messages() const override {
        return {};
    }

    void clear_messages() override {}
};

// ── Helpers ───────────────────────────────────────────────────────────────────

static VirtualDeviceModel make_mqtt_light() {
    return {"ikea_bulb_v1", "IKEA Bulb v1", "light", "mqtt", {"on_off", "brightness"}};
}

// ── state_topic ───────────────────────────────────────────────────────────────

TEST(VirtualDeviceMqttTest, StateTopic_Format) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("lamp_salon", "Lampe salon", "salon", &m);

    EXPECT_EQ(light.state_topic(), "home/salon/lamp_salon/state");
}

// ── command_topic ─────────────────────────────────────────────────────────────

TEST(VirtualDeviceMqttTest, CommandTopic_Format) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("light1", "Light 1", "chambre", &m);

    EXPECT_EQ(light.command_topic(), "home/chambre/light1");
}

TEST(VirtualDeviceMqttTest, CommandTopic_UsesRoom) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("lamp_salon", "Lampe salon", "salon", &m);

    EXPECT_EQ(light.command_topic(), "home/salon/lamp_salon");
}

TEST(VirtualDeviceMqttTest, Constructor_CreatesMqttClientAssociation) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("lamp_salon", "Lampe salon", "salon", &m);

    ASSERT_NE(light.protocol_client(), nullptr);
    EXPECT_EQ(light.protocol_client()->protocol(), AdapterProtocol::Mqtt);
}

TEST(VirtualDeviceMqttTest, Constructor_UsesSharedMqttClient) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light1("lamp_salon", "Lampe salon", "salon", &m);
    VirtualLight light2("lamp_chambre", "Lampe chambre", "chambre", &m);

    ASSERT_NE(light1.protocol_client(), nullptr);
    ASSERT_NE(light2.protocol_client(), nullptr);
    EXPECT_EQ(light1.protocol_client(), light2.protocol_client());
}

// ── publish_state ─────────────────────────────────────────────────────────────

TEST(VirtualDeviceMqttTest, PublishState_IsNoopWithoutClient) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("lamp_salon", "Lampe salon", "salon", &m);
    light.init_states();

    // Must not crash when no client is set.
    EXPECT_NO_THROW(light.publish_state());
}

TEST(VirtualDeviceMqttTest, PublishState_CallsPublishOnClient) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("lamp_salon", "Lampe salon", "salon", &m);
    light.init_states();

    MockMQTTClient mock;
    light.set_protocol_client(&mock);
    light.publish_state();

    EXPECT_EQ(mock.publish_call_count, 1);
    EXPECT_EQ(mock.last_topic, "home/salon/lamp_salon/state");
}

TEST(VirtualDeviceMqttTest, PublishState_PayloadContainsStates) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("lamp_salon", "Lampe salon", "salon", &m);
    light.init_states();
    light.set_state("on", "true");
    light.set_state("brightness", "75");

    MockMQTTClient mock;
    light.set_protocol_client(&mock);
    light.publish_state();

    // Payload must be valid JSON containing both keys.
    EXPECT_NE(mock.last_message.find("\"on\""),         std::string::npos);
    EXPECT_NE(mock.last_message.find("\"true\""),       std::string::npos);
    EXPECT_NE(mock.last_message.find("\"brightness\""), std::string::npos);
    EXPECT_NE(mock.last_message.find("\"75\""),         std::string::npos);
}

TEST(VirtualDeviceMqttTest, SetMqttClient_CanBeCleared) {
    VirtualDeviceModel m = make_mqtt_light();
    VirtualLight light("lamp_salon", "Lampe salon", "salon", &m);
    light.init_states();

    MockMQTTClient mock;
    light.set_protocol_client(&mock);
    light.set_protocol_client(nullptr);
    light.publish_state();

    EXPECT_EQ(mock.publish_call_count, 0);
}
