#include <gtest/gtest.h>

#include <stdexcept>

#include "rest/rest_client.hpp"
#include "rest/rest_server.hpp"
#include "rest/rest_servers_manager.hpp"

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

// ---- RestServersManager -----------------------------------------------------

TEST(RestServersManagerTest, AssignEndpointUsesBaseAndPortAndDeviceId) {
    RestServersManager manager(51000);

    const std::string endpoint = manager.assign_endpoint("dev-001", "http://127.0.0.1");

    EXPECT_EQ(endpoint, "http://127.0.0.1:51000/devices/dev-001");
}

TEST(RestServersManagerTest, AssignEndpointReturnsExistingValueForSameDevice) {
    RestServersManager manager(51000);

    const std::string first = manager.assign_endpoint("dev-001", "http://host");
    const std::string second = manager.assign_endpoint("dev-001", "http://other");

    EXPECT_EQ(first, second);
}

TEST(RestServersManagerTest, AssignEndpointIncrementsPortForNewDevice) {
    RestServersManager manager(51000);

    const std::string first = manager.assign_endpoint("dev-001", "http://host");
    const std::string second = manager.assign_endpoint("dev-002", "http://host");

    EXPECT_EQ(first, "http://host:51000/devices/dev-001");
    EXPECT_EQ(second, "http://host:51001/devices/dev-002");
}

TEST(RestServersManagerTest, EndpointForReturnsNulloptWhenDeviceIsUnknown) {
    RestServersManager manager;

    EXPECT_FALSE(manager.endpoint_for("missing").has_value());
}

TEST(RestServersManagerTest, HasDeviceRemoveAndEndpointForWorkTogether) {
    RestServersManager manager;
    manager.assign_endpoint("dev-001", "http://127.0.0.1");

    ASSERT_TRUE(manager.has_device("dev-001"));
    ASSERT_TRUE(manager.endpoint_for("dev-001").has_value());

    manager.remove_device("dev-001");

    EXPECT_FALSE(manager.has_device("dev-001"));
    EXPECT_FALSE(manager.endpoint_for("dev-001").has_value());
}

TEST(RestServersManagerTest, ClearRemovesAllAndResetsPortCursor) {
    RestServersManager manager(52000);
    manager.assign_endpoint("dev-001", "http://host");
    manager.assign_endpoint("dev-002", "http://host");

    manager.clear();

    EXPECT_FALSE(manager.has_device("dev-001"));
    EXPECT_FALSE(manager.has_device("dev-002"));

    const std::string endpoint = manager.assign_endpoint("dev-003", "http://host");
    EXPECT_EQ(endpoint, "http://host:52000/devices/dev-003");
}

TEST(RestServersManagerTest, AssignEndpointRejectsInvalidInput) {
    RestServersManager manager;

    EXPECT_THROW(manager.assign_endpoint("", "http://host"), std::invalid_argument);
    EXPECT_THROW(manager.assign_endpoint("dev-001", ""), std::invalid_argument);
}

TEST(RestServersManagerTest, ConstructorRejectsInvalidPortRange) {
    EXPECT_THROW(RestServersManager(0), std::invalid_argument);
    EXPECT_THROW(RestServersManager(65536), std::invalid_argument);
}
