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
    ('nest_thermostat', 'Nest Thermostat', 'thermostat', 'rest');

INSERT OR IGNORE INTO model_capabilities (model_id, capability) VALUES
    ('ikea_bulb_v1', 'on_off'),
    ('ikea_bulb_v1', 'brightness'),
    ('philips_hue_v2', 'on_off'),
    ('philips_hue_v2', 'brightness'),
    ('philips_hue_v2', 'color'),
    ('nest_thermostat', 'temperature'),
    ('nest_thermostat', 'target_temperature');

INSERT OR IGNORE INTO devices (id, label, room, model_id) VALUES
    ('lamp_salon', 'Lampe du salon', 'salon', 'ikea_bulb_v1'),
    ('lamp_chambre', 'Lampe chambre', 'chambre', 'philips_hue_v2');
