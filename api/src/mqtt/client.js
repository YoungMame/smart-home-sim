const mqtt = require('mqtt');
const { setState } = require('../state/store');

const BROKER_URL = process.env.MQTT_BROKER_URL || 'mqtt://localhost:1883';

// Topic pattern: home/:type/:id/state  (published by each device)
const STATE_TOPIC = 'home/+/+/state';

let client;
const stateListeners = new Set();

function connect() {
  client = mqtt.connect(BROKER_URL);

  client.on('connect', () => {
    console.log(`[MQTT] Connected to ${BROKER_URL}`);
    client.subscribe(STATE_TOPIC, { qos: 0 });
  });

  client.on('message', (topic, payload) => {
    // Expected format: home/:type/:id/state
    const parts = topic.split('/');
    if (parts.length !== 4 || parts[3] !== 'state') return;

    const [, type, id] = parts;
    let state;
    try {
      state = JSON.parse(payload.toString());
    } catch {
      return; // ignore malformed payloads
    }

    setState(type, id, state);
    stateListeners.forEach(fn => fn(type, id, state));
  });

  client.on('error', err => console.error('[MQTT] Error:', err.message));
  client.on('disconnect', () => console.warn('[MQTT] Disconnected from broker'));

  return client;
}

// Publish a command to a device: home/:type/:id/command
function publishCommand(type, id, command) {
  if (!client?.connected) {
    throw new Error('MQTT client not connected');
  }
  const topic = `home/${type}/${id}/command`;
  client.publish(topic, JSON.stringify(command), { qos: 0 });
}

// Register a listener called whenever a device state update arrives
function onStateUpdate(fn) {
  stateListeners.add(fn);
  return () => stateListeners.delete(fn); // returns unsubscribe function
}

module.exports = { connect, publishCommand, onStateUpdate };
