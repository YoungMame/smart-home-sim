const { Router } = require('express');
const { getState, getAllStates } = require('../state/store');
const { publishCommand } = require('../mqtt/client');

const router = Router();

// GET /devices — list all known device states
router.get('/', (_req, res) => {
  res.json(getAllStates());
});

// GET /devices/:type/:id — get state for one device
router.get('/:type/:id', (req, res) => {
  const { type, id } = req.params;
  const state = getState(type, id);
  if (!state) {
    return res.status(404).json({ error: 'Device not found or no state received yet' });
  }
  res.json({ type, id, state });
});

// POST /devices/:type/:id — send a command to a device
router.post('/:type/:id', (req, res) => {
  const { type, id } = req.params;
  const command = req.body;

  if (!command || typeof command !== 'object' || Array.isArray(command)) {
    return res.status(400).json({ error: 'Request body must be a JSON object' });
  }

  try {
    publishCommand(type, id, command);
    res.json({ ok: true, type, id, command });
  } catch (err) {
    res.status(503).json({ error: err.message });
  }
});

module.exports = router;
