const express = require('express');
const mqttClient = require('./mqtt/client');
const wsServer = require('./ws/server');
const devicesRouter = require('./routes/devices');

const HTTP_PORT = parseInt(process.env.HTTP_PORT || '3000', 10);

// 1. Connect to the MQTT broker
mqttClient.connect();

// 2. Start the WebSocket server on :8080
wsServer.start();

// 3. Set up the Express REST API on :3000
const app = express();
app.use(express.json());

app.get('/health', (_req, res) => res.json({ status: 'ok' }));
app.use('/devices', devicesRouter);

app.listen(HTTP_PORT, () => {
  console.log(`[HTTP] REST API listening on :${HTTP_PORT}`);
});
