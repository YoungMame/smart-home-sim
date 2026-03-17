#include <gtest/gtest.h>

#include <stdexcept>

#include "ip_manager/ip_manager.hpp"
#include "rest/rest_client.hpp"
#include "rest/rest_server.hpp"

// ---- RestClient -------------------------------------------------------------

TEST(RestClientTest, IsDisconnectedByDefault) {
    RestClient client;

    EXPECT_FALSE(client.is_connected());
}

TEST(RestClientTest, ConnectAndDisconnectUpdatesConnectionState) {
    RestClient client;

    client.connect("http://127.0.0.1:8080");
    EXPECT_TRUE(client.is_connected());

    client.disconnect();
    EXPECT_FALSE(client.is_connected());
}

TEST(RestClientTest, GetThrowsWhenDisconnected) {
    RestClient client;

    EXPECT_THROW(client.get("/status"), std::runtime_error);
}

TEST(RestClientTest, GetRecordsRequest) {
    RestClient client;
    client.connect("http://127.0.0.1:8080");

    const SimulatedMessage req = client.get("/state", "{\"on\":true}");
    const auto all_requests = client.requests();

    ASSERT_EQ(all_requests.size(), 1u);
    EXPECT_EQ(req.protocol, AdapterProtocol::Rest);
    EXPECT_EQ(req.channel, "/state");
    EXPECT_EQ(req.payload, "{\"on\":true}");
    EXPECT_EQ(all_requests.front().channel, "/state");
}

TEST(RestClientTest, ClearRequestsRemovesAllRequests) {
    RestClient client;
    client.connect("http://127.0.0.1:8080");
    client.get("/a");
    client.get("/b");

    client.clear_requests();

    EXPECT_TRUE(client.requests().empty());
}

// ---- RestServer -------------------------------------------------------------

TEST(RestServerTest, KeepsDeviceIdAndProtocol) {
    RestServer server("dev-001");

    EXPECT_EQ(server.device_id(), "dev-001");
    EXPECT_EQ(server.protocol(), AdapterProtocol::Rest);
}

TEST(RestServerTest, ConnectRejectsEmptyEndpoint) {
    RestServer server("dev-001");

    EXPECT_THROW(server.connect(""), std::invalid_argument);
}

TEST(RestServerTest, SendThrowsWhenDisconnected) {
    RestServer server("dev-001");

    EXPECT_THROW(server.send("/state", "{}"), std::runtime_error);
}

TEST(RestServerTest, ConnectSendAndCollectMessages) {
    RestServer server("dev-001");
    server.connect("http://127.0.0.1:50000/devices/dev-001");

    server.send("/state", "{\"temp\":22}");

    const auto messages = server.messages();
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_TRUE(server.is_connected());
    EXPECT_EQ(messages.front().protocol, AdapterProtocol::Rest);
    EXPECT_EQ(messages.front().channel, "/state");
    EXPECT_EQ(messages.front().payload, "{\"temp\":22}");
}

TEST(RestServerTest, DisconnectIsIdempotent) {
    RestServer server("dev-001");

    EXPECT_NO_THROW(server.disconnect());

    server.connect("http://127.0.0.1:50000/devices/dev-001");
    EXPECT_TRUE(server.is_connected());

    EXPECT_NO_THROW(server.disconnect());
    EXPECT_FALSE(server.is_connected());
}

TEST(RestServerTest, ClearMessagesRemovesAllMessages) {
    RestServer server("dev-001");
    server.connect("http://127.0.0.1:50000/devices/dev-001");
    server.send("/a", "1");
    server.send("/b", "2");

    server.clear_messages();

    EXPECT_TRUE(server.messages().empty());
}

// ---- IpManager -------------------------------------------------------------

TEST(IpManagerTest, AssignIpByteReturnsFirstAvailableByte) {
    IpManager manager(43);

    const auto ip_byte = manager.assign_ip_byte("dev-001");

    EXPECT_EQ(ip_byte, static_cast<std::uint8_t>(43));
}

TEST(IpManagerTest, AssignIpByteReturnsExistingValueForSameDevice) {
    IpManager manager(10);

    const auto first = manager.assign_ip_byte("dev-001");
    const auto second = manager.assign_ip_byte("dev-001");

    EXPECT_EQ(first, second);
}

TEST(IpManagerTest, AssignIpByteIncrementsForNewDevice) {
    IpManager manager(20);

    const auto first = manager.assign_ip_byte("dev-001");
    const auto second = manager.assign_ip_byte("dev-002");

    EXPECT_EQ(first, static_cast<std::uint8_t>(20));
    EXPECT_EQ(second, static_cast<std::uint8_t>(21));
}

TEST(IpManagerTest, IpByteForReturnsNulloptWhenDeviceIsUnknown) {
    IpManager manager;

    EXPECT_FALSE(manager.ip_byte_for("missing").has_value());
}

TEST(IpManagerTest, HasDeviceRemoveAndIpByteForWorkTogether) {
    IpManager manager;
    manager.assign_ip_byte("dev-001");

    ASSERT_TRUE(manager.has_device("dev-001"));
    ASSERT_TRUE(manager.ip_byte_for("dev-001").has_value());

    manager.remove_device("dev-001");

    EXPECT_FALSE(manager.has_device("dev-001"));
    EXPECT_FALSE(manager.ip_byte_for("dev-001").has_value());
}

TEST(IpManagerTest, ClearRemovesAllAndResetsIpCursor) {
    IpManager manager(30);
    manager.assign_ip_byte("dev-001");
    manager.assign_ip_byte("dev-002");

    manager.clear();

    EXPECT_FALSE(manager.has_device("dev-001"));
    EXPECT_FALSE(manager.has_device("dev-002"));

    const auto ip_byte = manager.assign_ip_byte("dev-003");
    EXPECT_EQ(ip_byte, static_cast<std::uint8_t>(30));
}

TEST(IpManagerTest, AssignIpByteRejectsInvalidDeviceId) {
    IpManager manager;

    EXPECT_THROW(manager.assign_ip_byte(""), std::invalid_argument);
}

TEST(IpManagerTest, ConstructorRejectsInvalidIpByteRange) {
    EXPECT_THROW(IpManager(0), std::invalid_argument);
    EXPECT_THROW(IpManager(1), std::invalid_argument);
}

// Delete a device and check that its IP byte can be reassigned to a new device, ensuring proper cleanup of resources.

TEST(IpManagerTest, RemoveDeviceFreesIpByteForReassignment) {
    IpManager manager(50);

    const auto ip_byte1 = manager.assign_ip_byte("dev-001");
    EXPECT_EQ(ip_byte1, static_cast<std::uint8_t>(50));

    manager.remove_device("dev-001");

    const auto ip_byte2 = manager.assign_ip_byte("dev-002");
    EXPECT_EQ(ip_byte2, static_cast<std::uint8_t>(50));
}
