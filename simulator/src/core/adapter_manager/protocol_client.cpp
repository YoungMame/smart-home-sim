#include "protocol_client.hpp"

std::string to_string(AdapterProtocol protocol) {
    switch (protocol) {
    case AdapterProtocol::Rest:
        return "rest";
    case AdapterProtocol::Ws:
        return "ws";
    case AdapterProtocol::Mqtt:
        return "mqtt";
    }

    return "unknown";
}
