import { useCallback, useEffect, useRef, useState } from 'react';
import * as api from '../api/client.js';

// All game state on the client. The server owns the rules; this hook only
// keeps what it needs to draw the screen and sends input to the API.
//
// TIP: follow a guess here -> api.sendGuess -> Server.cpp "/api/game/guess"
//      -> GameSession::makeGuess (GUIDE: Part 3, "Life of a guess").

const WORD_LENGTH = 5;

const initialState = {
  gameId: null,
  difficulty: 'normal',
  maxAttempts: 6,
  rows: [], // [{ guess, feedback }]
  current: '',
  status: 'idle', // idle | playing | won | lost
  score: 0,
  secretWord: '',
  letterStatus: {}, // { A: 'correct', ... } from the server's LetterTracker
  hints: [], // [{ position, letter }]
  hintsUsed: 0,
  lookup: null, // { word, found, methods } for the last word sent (even a rejected one)
  possibleWords: null,
  message: '',
  error: false,
  shake: false,
  version: 0, // bumped after every accepted change, so panels can refresh
};

// method: lookup used to validate guesses (see components/MethodSelect.jsx)
export default function useGame(method = 'hash') {
  const [game, setGame] = useState(initialState);
  const busy = useRef(false);

  const flash = useCallback((message, error = false, shake = false) => {
    setGame((g) => ({ ...g, message, error, shake }));
    if (shake) setTimeout(() => setGame((g) => ({ ...g, shake: false })), 400);
  }, []);

  const start = useCallback(async (difficulty) => {
    try {
      const res = await api.newGame(difficulty);
      setGame({
        ...initialState,
        gameId: res.gameId,
        difficulty: res.difficulty,
        maxAttempts: res.maxAttempts,
        status: 'playing',
        message: `New ${res.difficulty} game - ${res.maxAttempts} attempts`,
        version: Date.now(),
      });
    } catch (e) {
      flash(e.message, true);
    }
  }, [flash]);

  const addLetter = useCallback((letter) => {
    setGame((g) =>
      g.status !== 'playing' || g.current.length >= WORD_LENGTH
        ? g
        : { ...g, current: g.current + letter.toUpperCase(), message: '', error: false }
    );
  }, []);

  const removeLetter = useCallback(() => {
    setGame((g) => (g.status !== 'playing' ? g : { ...g, current: g.current.slice(0, -1) }));
  }, []);

  const submit = useCallback(async () => {
    const g = game;
    if (g.status !== 'playing' || busy.current) return;
    if (g.current.length < WORD_LENGTH) {
      flash('Not enough letters', true, true);
      return;
    }

    busy.current = true;
    try {
      const res = await api.sendGuess(g.gameId, g.current, method);
      setGame((prev) => ({
        ...prev,
        rows: [...prev.rows, { guess: res.guess, feedback: res.feedback }],
        current: '',
        status: res.status,
        score: res.score,
        secretWord: res.secretWord,
        letterStatus: res.letterStatus ?? prev.letterStatus,
        lookup: { word: res.guess, found: true, methods: res.lookupMethods },
        possibleWords: res.possibleWords,
        message:
          res.status === 'won'
            ? `Solved! Score ${res.score}`
            : res.status === 'lost'
              ? `Out of attempts - the word was ${res.secretWord}`
              : `${res.attemptsLeft} attempts left`,
        error: false,
        version: prev.version + 1,
      }));
    } catch (e) {
      // Rejected guesses still carry lookupStats: show how each method missed.
      setGame((prev) => ({
        ...prev,
        lookup: e.data?.lookupMethods
          ? { word: prev.current, found: false, methods: e.data.lookupMethods }
          : prev.lookup,
      }));
      flash(e.message, true, true);
    } finally {
      busy.current = false;
    }
  }, [game, flash, method]);

  const requestHint = useCallback(async () => {
    if (game.status !== 'playing') return;
    try {
      const res = await api.getHint(game.gameId);
      setGame((g) => ({
        ...g,
        hints: [...g.hints, { position: res.position, letter: res.letter }],
        hintsUsed: res.hintsUsed,
        message: `Hint: position ${res.position + 1} is ${res.letter} (-50 points)`,
        error: false,
        version: g.version + 1,
      }));
    } catch (e) {
      flash(e.message, true);
    }
  }, [game.status, game.gameId, flash]);

  // Physical keyboard: letters, Enter, Backspace.
  useEffect(() => {
    const onKey = (e) => {
      if (e.ctrlKey || e.metaKey || e.altKey) return;
      if (e.target instanceof HTMLInputElement) return; // typing a name / search
      if (e.key === 'Enter') submit();
      else if (e.key === 'Backspace') removeLetter();
      else if (/^[a-zA-Z]$/.test(e.key)) addLetter(e.key);
    };
    window.addEventListener('keydown', onKey);
    return () => window.removeEventListener('keydown', onKey);
  }, [submit, removeLetter, addLetter]);

  return { game, start, addLetter, removeLetter, submit, requestHint };
}
