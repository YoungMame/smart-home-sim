#include "event_engine/event_engine.hpp"

#include <iostream>

#include <nlohmann/json.hpp>

#include "device_engine/device_engine.hpp"
#include "scheduler/event_scheduler.hpp"

using json = nlohmann::json;

// ── Singleton ────────────────────────────────────────────────────────────────

EventEngine& EventEngine::instance() {
    static EventEngine inst;
    return inst;
}

namespace {
bool is_builtin_event_type(const std::string& type) {
    return type == "state_change" || type == "trigger";
}
}

// Helpers

bool EventEngine::is_known_device(const std::string& device_id) const {
    return DeviceEngine::instance().has_device(device_id);
}

// ── Public API ───────────────────────────────────────────────────────────────

void EventEngine::process_event(const Event& event) {
    if (!is_builtin_event_type(event.type) && !is_known_event_type(event.type)) {
        std::cerr << "[EventEngine] Unknown event type: '" << event.type << "'\n";
        return;
    }

    if (!event.device_id.empty() && !is_known_device(event.device_id)) {
        std::cerr << "[EventEngine] Unknown device_id: '" << event.device_id << "'\n";
        return;
    }

    if (event.type == "state_change") {
        handle_state_change(event);
    } else if (event.type == "trigger") {
        handle_trigger(event);
    } else {
        handle_single_event(event);
    }
}

void EventEngine::add_event_handler(const std::string& device_id, const std::string& event_type, std::function<void(const Event&)> handler) {
    if (!is_builtin_event_type(event_type) && !is_known_event_type(event_type)) {
        std::cerr << "[EventEngine] Refusing handler registration for unknown event type: '"
                  << event_type << "'\n";
        return;
    }

    custom_handlers_[device_id].handlers[event_type] = std::move(handler);
}

// ── Handlers ─────────────────────────────────────────────────────────────────

// Applies a state change to the target device.
// Expected payload: JSON object of key/value pairs, e.g. {"on":"true","brightness":"80"}
void EventEngine::handle_state_change(const Event& event) {
    if (event.device_id.empty()) {
        std::cerr << "[EventEngine] state_change event has no device_id — skipping\n";
        return;
    }

    DeviceEngine::instance().update_device_state(event.device_id, event);
}

// API handler
// Expected payload: JSON object with an "events" array, e.g.
// event.payload = R"({
//   "events": [
//     {"type": "state_change", "device_id": "light1", "payload": {"on": "true"}, "delay_ms": 0},
//     {"type": "state_change", "device_id": "thermo1", "payload": {"mode": "heat", "temp": "22"}, "delay_ms": 500},
//     {"type": "trigger", "payload": {"events": [ ... ]}, "delay_ms": 1000}
//   ]
// })";
void EventEngine::handle_trigger(const Event& event) {
    if (event.payload.empty()) return;

    try {
        const json payload = json::parse(event.payload);

        if (!payload.contains("events") || !payload.at("events").is_array()) return;

        for (const auto& sub : payload.at("events")) {
            Event e;
            e.type      = sub.value("type",      "state_change");
            e.device_id = sub.value("device_id", "");
            e.payload   = sub.contains("payload") ? sub.at("payload").dump() : "";
            e.scheduled_at = std::chrono::steady_clock::now()
                           + std::chrono::milliseconds(sub.value("delay_ms", 0));

            EventScheduler::instance().schedule_event(std::move(e));
        }
    } catch (const json::exception& ex) {
        std::cerr << "[EventEngine] Failed to parse trigger payload: " << ex.what() << "\n";
    }
}

void EventEngine::handle_single_event(const Event& event) {
    const auto& handlers_it = custom_handlers_.find(event.device_id);
    if (handlers_it == custom_handlers_.end()) {
        std::cerr << "[EventEngine] No handlers found for device_id: '" << event.device_id << "'\n";
        return;
    }

    const auto& event_handler_it = handlers_it->second.handlers.find(event.type);
    if (event_handler_it == handlers_it->second.handlers.end()) {
        std::cerr << "[EventEngine] No handler for event type '" << event.type << "' on device_id: '" << event.device_id << "'\n";
        return;
    }

    event_handler_it->second(event);
}
