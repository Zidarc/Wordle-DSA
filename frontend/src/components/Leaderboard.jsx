import { useEffect, useState } from 'react';
import { getLeaderboard } from '../api/client.js';

// Top 10 from data/scores.txt: streamed through a MinHeap of size 10, then
// merge sorted (Leaderboard.cpp). The search box filters names with KMP.
export default function Leaderboard({ version }) {
  const [rows, setRows] = useState([]);
  const [search, setSearch] = useState('');

  useEffect(() => {
    const t = setTimeout(() => {
      getLeaderboard(search.trim())
        .then(setRows)
        .catch(() => setRows([]));
    }, 250); // small debounce while typing
    return () => clearTimeout(t);
  }, [search, version]);

  return (
    <div className="panel">
      <h2>Leaderboard</h2>
      <span className="ds-tag">MinHeap top-10 + merge sort, KMP name search</span>
      <input
        className="select search"
        value={search}
        placeholder="search name (KMP)"
        onChange={(e) => setSearch(e.target.value)}
      />
      {rows.length === 0 ? (
        <p className="muted">No scores yet.</p>
      ) : (
        <table className="data">
          <thead>
            <tr>
              <th>#</th>
              <th>Name</th>
              <th>Level</th>
              <th className="num">Score</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((r) => (
              <tr key={`${r.rank}-${r.name}-${r.date}`}>
                <td className="mono">{r.rank}</td>
                <td>{r.name}</td>
                <td>{r.difficulty}</td>
                <td className="num">{r.score}</td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </div>
  );
}
