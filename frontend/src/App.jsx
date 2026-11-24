import { useEffect, useRef, useState } from 'react';
import Board from './components/Board.jsx';
import DifficultySelect from './components/DifficultySelect.jsx';
import DsaInsights from './components/DsaInsights.jsx';
import HintButton from './components/HintButton.jsx';
import HistoryPanel from './components/HistoryPanel.jsx';
import Keyboard from './components/Keyboard.jsx';
import MethodSelect from './components/MethodSelect.jsx';
import SidePanel from './components/SidePanel.jsx';
import useGame from './hooks/useGame.js';

// App layout: top bar (difficulty, new game), the game column on the left,
// and the DSA side panels on the right.
// TIP: all game state is in hooks/useGame.js; all fetches in api/client.js.
export default function App() {
  const [method, setMethod] = useState('hash');
  const { game, start, addLetter, removeLetter, submit, requestHint } = useGame(method);
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
          <MethodSelect value={method} onChange={setMethod} />
          <DifficultySelect value={difficulty} onChange={setDifficulty} />
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
          <HintButton
            hints={game.hints}
            hintsUsed={game.hintsUsed}
            onHint={requestHint}
            disabled={game.status !== 'playing'}
          />
          <Keyboard
            letterStatus={game.letterStatus}
            onLetter={addLetter}
            onEnter={submit}
            onBackspace={removeLetter}
            disabled={game.status !== 'playing'}
          />
        </section>
        <SidePanel
          tabs={[
            {
              id: 'insights',
              label: 'DSA Insights',
              render: () => <DsaInsights lookup={game.lookup} method={method} version={game.version} />,
            },
            {
              id: 'history',
              label: 'History',
              render: () => <HistoryPanel gameId={game.gameId} version={game.version} />,
            },
          ]}
        />
      </div>
    </div>
  );
}
