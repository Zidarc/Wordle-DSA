import { useEffect, useRef, useState } from 'react';
import Board from './components/Board.jsx';
import useGame from './hooks/useGame.js';

// App layout: top bar (difficulty, new game), the game column on the left,
// and the DSA side panels on the right.
// TIP: all game state is in hooks/useGame.js; all fetches in api/client.js.
export default function App() {
  const { game, start, submit } = useGame();
  const [difficulty, setDifficulty] = useState('normal');
  const started = useRef(false);

  // Start one game on first load (the ref stops StrictMode's double effect).
  useEffect(() => {
    if (started.current) return;
    started.current = true;
    start('normal');
  }, [start]);

  return (
    <div className="app">
      <header className="topbar">
        <div className="brand">
          <h1>Wordle-DSA</h1>
          <span>hand-built data structures</span>
        </div>
        <div className="controls">
          <select className="select" value={difficulty} onChange={(e) => setDifficulty(e.target.value)}>
            <option value="easy">Easy - 8 tries</option>
            <option value="normal">Normal - 6 tries</option>
            <option value="hard">Hard - 5 tries</option>
          </select>
          <button className="btn primary" onClick={() => start(difficulty)}>
            New game
          </button>
        </div>
      </header>

      <div className="layout">
        <section className="game-column">
          <div className={`status-line${game.error ? ' error' : ''}`}>{game.message}</div>
          <Board
            rows={game.rows}
            current={game.current}
            maxAttempts={game.maxAttempts}
            shake={game.shake}
          />
          <button className="btn" onClick={submit} disabled={game.status !== 'playing'}>
            Enter
          </button>
        </section>
        <aside />
      </div>
    </div>
  );
}
