PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS device_models (
    id TEXT PRIMARY KEY,
    label TEXT NOT NULL,
    type TEXT NOT NULL,
    protocol TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS model_capabilities (
    model_id TEXT NOT NULL,
    capability TEXT NOT NULL,
    PRIMARY KEY (model_id, capability),
    FOREIGN KEY (model_id) REFERENCES device_models(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS model_available_events (
    model_id TEXT NOT NULL,
    event_type TEXT NOT NULL,
    PRIMARY KEY (model_id, event_type),
    FOREIGN KEY (model_id) REFERENCES device_models(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS devices (
    id TEXT PRIMARY KEY,
    label TEXT NOT NULL,
    room TEXT NOT NULL,
    model_id TEXT NOT NULL,
    FOREIGN KEY (model_id) REFERENCES device_models(id)
);

INSERT OR IGNORE INTO device_models (id, label, type, protocol) VALUES
    ('ikea_bulb_v1', 'IKEA Bulb v1', 'light', 'mqtt'),
    ('philips_hue_v2', 'Philips Hue v2', 'light', 'rest'),
    ('nest_thermostat', 'Nest Thermostat', 'thermostat', 'rest'),
    ('aqara_motion_p1', 'Aqara Motion P1', 'motion', 'mqtt'),
    ('eve_door_window', 'Eve Door & Window', 'contact', 'rest'),
    ('ikea_remote_btn', 'IKEA Remote Button', 'button', 'mqtt'),
    ('tapo_plug_energy', 'Tapo Plug Energy', 'energy', 'mqtt');

INSERT OR IGNORE INTO model_capabilities (model_id, capability) VALUES
    ('ikea_bulb_v1', 'on_off'),
    ('ikea_bulb_v1', 'brightness'),
    ('philips_hue_v2', 'on_off'),
    ('philips_hue_v2', 'brightness'),
    ('philips_hue_v2', 'color'),
    ('nest_thermostat', 'temperature'),
    ('nest_thermostat', 'target_temperature'),
    ('aqara_motion_p1', 'motion'),
    ('aqara_motion_p1', 'presence'),
    ('eve_door_window', 'contact'),
    ('eve_door_window', 'lock'),
    ('ikea_remote_btn', 'button_input'),
    ('tapo_plug_energy', 'power_meter'),
    ('tapo_plug_energy', 'energy_meter');

INSERT OR IGNORE INTO model_available_events (model_id, event_type) VALUES
    -- Correlation with EventTypes.light
    ('ikea_bulb_v1', 'light.turned_on'),
    ('ikea_bulb_v1', 'light.turned_off'),
    ('ikea_bulb_v1', 'light.brightness_changed'),
    ('philips_hue_v2', 'light.turned_on'),
    ('philips_hue_v2', 'light.turned_off'),
    ('philips_hue_v2', 'light.brightness_changed'),
    ('philips_hue_v2', 'light.color_changed'),
    -- Correlation with EventTypes.thermostat
    ('nest_thermostat', 'thermostat.temperature_changed'),
    ('nest_thermostat', 'thermostat.setpoint_changed'),
    ('nest_thermostat', 'thermostat.mode_changed'),
    ('nest_thermostat', 'thermostat.humidity_changed'),
    -- Correlation with EventTypes.motion
    ('aqara_motion_p1', 'motion.motion_detected'),
    ('aqara_motion_p1', 'motion.motion_cleared'),
    ('aqara_motion_p1', 'motion.presence_detected'),
    ('aqara_motion_p1', 'motion.presence_cleared'),
    -- Correlation with EventTypes.contact
    ('eve_door_window', 'contact.opened'),
    ('eve_door_window', 'contact.closed'),
    ('eve_door_window', 'contact.locked'),
    ('eve_door_window', 'contact.unlocked'),
    -- Correlation with EventTypes.button
    ('ikea_remote_btn', 'button.single_press'),
    ('ikea_remote_btn', 'button.double_press'),
    ('ikea_remote_btn', 'button.long_press'),
    ('ikea_remote_btn', 'button.released'),
    -- Correlation with EventTypes.energy
    ('tapo_plug_energy', 'energy.power_changed'),
    ('tapo_plug_energy', 'energy.consumption_updated'),
    ('tapo_plug_energy', 'energy.overload_detected');

INSERT OR IGNORE INTO devices (id, label, room, model_id) VALUES
    ('lamp_salon', 'Lampe du salon', 'salon', 'ikea_bulb_v1'),
    ('lamp_chambre', 'Lampe chambre', 'chambre', 'philips_hue_v2');
