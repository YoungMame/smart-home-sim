#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include "../event_engine/event.hpp"

// EventScheduler — Singleton.
// Owns the simulation's main loop: events are pushed from anywhere in the
// codebase via schedule_event() and dispatched in chronological order by run().
class EventScheduler {
public:
    // Returns the unique instance (constructed on first call, thread-safe in C++11).
    static EventScheduler& instance();

    // Non-copyable, non-movable.
    EventScheduler(const EventScheduler&)            = delete;
    EventScheduler& operator=(const EventScheduler&) = delete;
    EventScheduler(EventScheduler&&)                 = delete;
    EventScheduler& operator=(EventScheduler&&)      = delete;

    // Thread-safe: enqueue an event; run() will dispatch it at event.scheduled_at.
    void schedule_event(Event event);

    // Blocking: starts the main loop; returns only after stop() is called.
    void run();

    // Signal the main loop to exit cleanly (safe to call from a signal handler).
    void stop();

private:
    EventScheduler() = default;

    using EventQueue = std::priority_queue<Event, std::vector<Event>, EventComparator>;

    EventQueue              queue_;
    std::mutex              mutex_;
    std::condition_variable cv_;
    std::atomic<bool>       running_{false};
};
