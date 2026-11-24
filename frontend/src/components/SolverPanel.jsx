import { useEffect, useState } from 'react';
import { getSolver } from '../api/client.js';

// Solver: words still possible (linked list), the backtracking search run two
// ways (recursion vs explicit ArrayStack), and quick-sorted suggestions.
// TIP: server code is backend/src/Solver.cpp (GUIDE: Part 2, backtracking).
export default function SolverPanel({ gameId, version }) {
  const [data, setData] = useState(null);
  const [error, setError] = useState('');

  useEffect(() => {
    if (!gameId) return;
    getSolver(gameId, 5)
      .then((d) => {
        setData(d);
        setError('');
      })
      .catch((e) => setError(e.message));
  }, [gameId, version]);

  if (error) return <div className="panel status-line error">{error}</div>;
  if (!data) return <div className="panel muted">Loading solver...</div>;

  const rows = [
    ['Letters tried', 'nodesVisited'],
    ['Branches pruned', 'pruned'],
    ['Prefix binary-search comparisons', 'prefixChecks'],
    ['Max depth / stack size', 'maxDepth'],
    ['Words found', 'solutions'],
  ];

  return (
    <div className="panel">
      <h2>Solver</h2>
      <span className="ds-tag">SinglyLinkedList + backtracking + quick sort</span>
      <p className="big-number mono">{data.possibleWords}</p>
      <p className="muted">words still fit every guess</p>

      <table className="data">
        <thead>
          <tr>
            <th>Backtracking</th>
            <th className="num">recursive</th>
            <th className="num">explicit stack</th>
          </tr>
        </thead>
        <tbody>
          {rows.map(([label, key]) => (
            <tr key={key}>
              <td>{label}</td>
              <td className="num">{data.backtracking[key]}</td>
              <td className="num">{data.backtrackingIterative[key]}</td>
            </tr>
          ))}
        </tbody>
      </table>
      <p className="muted small">Without pruning: 26^5 = 11,881,376 words to try.</p>

      <h2>Suggestions</h2>
      {data.suggestions.length === 0 ? (
        <p className="muted">Shown after 2 guesses (letter-frequency score, ranked by quick sort).</p>
      ) : (
        <ol className="mono">
          {data.suggestions.map((s) => (
            <li key={s.word}>
              {s.word} <span className="muted">score {s.score}</span>
            </li>
          ))}
        </ol>
      )}
    </div>
  );
}
