import { useEffect, useState } from 'react';
import { getHistory } from '../api/client.js';

// Guess history from the server's Stack<std::string>: most recent on top
// (the stack is read top -> bottom without popping).
export default function HistoryPanel({ gameId, version }) {
  const [history, setHistory] = useState([]);

  useEffect(() => {
    if (!gameId) return;
    getHistory(gameId)
      .then((res) => setHistory(res.history))
      .catch(() => setHistory([]));
  }, [gameId, version]);

  return (
    <div className="panel">
      <h2>Guess history</h2>
      <span className="ds-tag">Stack&lt;string&gt; - LIFO, top first</span>
      {history.length === 0 ? (
        <p className="muted">No guesses yet.</p>
      ) : (
        <ol className="stack-list mono">
          {history.map((word, i) => (
            <li key={i} className={i === 0 ? 'top' : ''}>
              {word} {i === 0 && <span className="muted">&larr; top</span>}
            </li>
          ))}
        </ol>
      )}
    </div>
  );
}
