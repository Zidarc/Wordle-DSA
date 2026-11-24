// All calls to the C++ backend live here (GUIDE: Part 3 explains each route).
// Vite proxies /api to http://localhost:8080 (see vite.config.js).
//
// Every function returns the parsed JSON. A non-2xx response throws an
// ApiError whose .data holds the JSON body, because the server sends useful
// fields with errors too (e.g. lookupStats for a word not in the list).

export class ApiError extends Error {
  constructor(message, status, data) {
    super(message);
    this.status = status;
    this.data = data;
  }
}

async function request(path, options = {}) {
  let response;
  try {
    response = await fetch(path, {
      headers: { 'Content-Type': 'application/json' },
      ...options,
    });
  } catch {
    throw new ApiError('Cannot reach the server. Is wordle_server running on port 8080?', 0, null);
  }

  let data = null;
  try {
    data = await response.json();
  } catch {
    // empty or non-JSON body
  }

  if (!response.ok) {
    throw new ApiError(data?.error ?? `Request failed (${response.status})`, response.status, data);
  }
  return data;
}

const post = (path, body) => request(path, { method: 'POST', body: JSON.stringify(body) });
const q = encodeURIComponent;

// ----- game (GameSession on the server) -----
export const newGame = (difficulty) => post('/api/game/new', { difficulty });
export const sendGuess = (gameId, guess, method = 'hash') =>
  post('/api/game/guess', { gameId, guess, method });
export const getHint = (gameId) => post('/api/game/hint', { gameId });
export const getHistory = (gameId) => request(`/api/game/history?gameId=${q(gameId)}`);
export const getReplay = (gameId) => request(`/api/game/replay?gameId=${q(gameId)}`);
export const getSolver = (gameId, count = 5) =>
  request(`/api/game/solver?gameId=${q(gameId)}&count=${count}`);

// ----- leaderboard (MinHeap + merge sort + KMP) -----
export const getLeaderboard = (search = '') =>
  request(`/api/leaderboard${search ? `?search=${q(search)}` : ''}`);
export const saveScore = (gameId, name) => post('/api/leaderboard', { gameId, name });

// ----- DSA panels -----
export const getDsaStats = () => request('/api/dsa/stats');
export const lookupWord = (word, method = 'hash') =>
  request(`/api/dsa/lookup?word=${q(word)}&method=${q(method)}`);
export const getTreeStats = () => request('/api/dsa/trees');
export const runSortingLab = (limit = 0) =>
  request(`/api/dsa/sort${limit ? `?limit=${limit}` : ''}`);
export const getRecentGames = () => request('/api/games/recent');
