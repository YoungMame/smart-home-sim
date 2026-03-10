#include "event_scheduler.hpp"

#include <iostream>

// TODO: uncomment once EventEngine is implemented.
// #include "../event_engine/event_engine.hpp"

EventScheduler& EventScheduler::instance() {
    static EventScheduler inst;
    return inst;
}

void EventScheduler::schedule_event(Event event) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(event));
    }
    cv_.notify_one();
}

void EventScheduler::run() {
    running_ = true;
    std::cout << "[EventScheduler] Main loop started.\n";

    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this] {
            return !queue_.empty() || !running_;
        });

        if (!running_) break;
        if (queue_.empty()) continue;

        const auto due = queue_.top().scheduled_at;
        const auto now = std::chrono::steady_clock::now();

        if (due > now) {
            // Wait until the event is due; wake up early if a newer (earlier) event
            // arrives or stop is requested.
            cv_.wait_until(lock, due, [this, &due] {
                return !running_ ||
                       (!queue_.empty() && queue_.top().scheduled_at < due);
            });
            continue;
        }

        Event event = queue_.top();
        queue_.pop();
        lock.unlock();

        // TODO: EventEngine::instance().process_event(event);
        std::cout << "[EventScheduler] Dispatching event: type=" << event.type
                  << " device=" << event.device_id << "\n";
    }

    std::cout << "[EventScheduler] Main loop stopped.\n";
}

void EventScheduler::stop() {
    running_ = false;
    cv_.notify_all();
}
