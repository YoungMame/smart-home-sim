const { WebSocketServer } = require('ws');
const { onStateUpdate } = require('../mqtt/client');

const WS_PORT = parseInt(process.env.WS_PORT || '8080', 10);

function start() {
  const wss = new WebSocketServer({ port: WS_PORT });

  wss.on('listening', () => {
    console.log(`[WS]   WebSocket server listening on :${WS_PORT}`);
  });

  // Broadcast every device state update to all connected clients
  onStateUpdate((type, id, state) => {
    const message = JSON.stringify({ type, id, state });
    wss.clients.forEach(ws => {
      if (ws.readyState === ws.OPEN) ws.send(message);
    });
  });

  return wss;
}

module.exports = { start };
