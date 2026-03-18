#pragma once

#include <functional>
#include <map>

#include "event.hpp"

// Store handlers by event type for a given device. The "state_change" and "trigger" types are reserved for built-in handlers.
struct EventHandlersContainer {
    std::unordered_map<std::string, std::function<void(const Event&)>> handlers;
};

// EventEngine — Singleton.
// Receives dispatched events from the EventScheduler and routes them
// to the appropriate handler (DeviceEngine, logger, etc.).
class EventEngine {
public:
    static EventEngine& instance();

    EventEngine(const EventEngine&)            = delete;
    EventEngine& operator=(const EventEngine&) = delete;
    EventEngine(EventEngine&&)                 = delete;
    EventEngine& operator=(EventEngine&&)      = delete;

    // Public API
    void process_event(const Event& event);

    // Event handling for devices classes
    void add_event_handler(const std::string& device_id, const std::string& event_type, std::function<void(const Event&)> handler);

private:
    EventEngine() = default;

    void handle_state_change(const Event& event);
    void handle_trigger(const Event& event);
    void handle_single_event(const Event& event);

    bool is_known_device(const std::string& device_id) const;

    std::map<std::string, EventHandlersContainer> custom_handlers_;

};
