#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Registry of all known simulation event categories and their sub-types.
// Used by EventEngine to validate incoming events.
//
// Categories map to real device capabilities exposed over MQTT, REST, or Matter.
// Each sub-type corresponds to a discrete physical action or sensor reading
// that a device can originate (user interaction, automation, or environment).
//
// When adding a new device type, add its events here first.
inline const std::unordered_map<std::string, std::unordered_set<std::string>> EventTypes = {

    // ── Lighting (on/off clusters, level control, color control) ─────────────
    // Sources: wall switch press, app command, automation rule, daylight sensor
    { "light", {
        "turned_on",          // light switched on (manually or via automation)
        "turned_off",         // light switched off
        "brightness_changed", // dimmer adjusted (0–100)
        "color_changed",      // RGB or color temperature changed
    }},

    // ── Thermostat (temperature measurement + HVAC control) ──────────────────
    // Sources: temperature sensor reading, user setpoint change, schedule
    { "thermostat", {
        "temperature_changed",  // ambient temperature reading updated
        "setpoint_changed",     // user changed target temperature
        "mode_changed",         // heat / cool / auto / off
        "humidity_changed",     // relative humidity reading updated
    }},

    // ── Motion & presence sensors ─────────────────────────────────────────────
    // Sources: PIR sensor, mmWave radar, camera, contact sensor
    { "motion", {
        "motion_detected",    // movement detected in zone
        "motion_cleared",     // no movement for cooldown period
        "presence_detected",  // person present (longer-duration presence)
        "presence_cleared",   // room now empty
    }},

    // ── Door / window contact sensors ─────────────────────────────────────────
    { "contact", {
        "opened",             // door or window opened
        "closed",             // door or window closed
        "locked",             // smart lock engaged
        "unlocked",           // smart lock released
    }},

    // ── Buttons & switches (stateless input devices) ──────────────────────────
    // Sources: smart button, remote control, wall switch
    { "button", {
        "single_press",       // short press
        "double_press",       // two quick presses
        "long_press",         // held down
        "released",           // button released after long press
    }},

    // ── Energy & power monitoring ─────────────────────────────────────────────
    // Sources: smart plug, energy meter
    { "energy", {
        "power_changed",      // watt reading updated
        "consumption_updated",// kWh total updated
        "overload_detected",  // power draw exceeded threshold
    }},

    // ── Scheduler / timer (internal simulation clock) ─────────────────────────
    { "timer", {
        "alarm",              // scheduled alarm fired
        "tick",               // periodic simulation tick
    }},
};

inline bool is_known_event_type(const std::string& type) {
    const std::size_t dot = type.find('.');
    if (dot == std::string::npos || dot == 0 || dot + 1 >= type.size()) {
        return false;
    }

    const std::string category = type.substr(0, dot);
    const std::string subtype = type.substr(dot + 1);

    const auto it = EventTypes.find(category);
    if (it == EventTypes.end()) {
        return false;
    }

    return it->second.count(subtype) > 0;
}

struct Event {
    std::string                               type;         // e.g. "state_change", "trigger", "light.turned_on"
    std::string                               device_id;    // target device (empty = broadcast)
    std::string                               payload;      // JSON-encoded data
    std::chrono::steady_clock::time_point     scheduled_at; // when the event should be dispatched
};

struct EventComparator {
    bool operator()(const Event& a, const Event& b) const {
        return a.scheduled_at > b.scheduled_at;
    }
};
