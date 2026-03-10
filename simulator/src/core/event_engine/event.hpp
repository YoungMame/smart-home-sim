#pragma once

#include <chrono>
#include <string>

// Represents a single discrete event to be processed by the simulation.
struct Event {
    std::string                               type;         // e.g. "state_change", "timer", "trigger"
    std::string                               device_id;    // target device (empty = broadcast)
    std::string                               payload;      // JSON-encoded data
    std::chrono::steady_clock::time_point     scheduled_at; // when the event should be dispatched
};

// Min-heap comparator: earliest scheduled_at = highest priority.
struct EventComparator {
    bool operator()(const Event& a, const Event& b) const {
        return a.scheduled_at > b.scheduled_at;
    }
};
