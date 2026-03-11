#pragma once

#include "event.hpp"

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

private:
    EventEngine() = default;

    void handle_state_change(const Event& event);
    void handle_trigger(const Event& event);

    bool is_known_device(const std::string& device_id) const;

};
