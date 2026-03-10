// In-memory cache: key = "type/id", value = merged state object
const states = new Map();

function getState(type, id) {
  return states.get(`${type}/${id}`) ?? null;
}

function setState(type, id, patch) {
  const key = `${type}/${id}`;
  states.set(key, { ...states.get(key), ...patch });
  return states.get(key);
}

function getAllStates() {
  return Object.fromEntries(states);
}

module.exports = { getState, setState, getAllStates };
