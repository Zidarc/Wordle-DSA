import { useEffect, useState } from 'react';
import { getRecentGames, getTreeStats, runSortingLab } from '../api/client.js';
import { formatTime } from './LookupTable.jsx';

// DSA Lab: Sorting Lab (SortingLab.cpp), tree properties
// (BinaryTreeProperties.h / BTree.h) and recent games (circular ArrayQueue).
export default function LabPanel({ version }) {
  const [sort, setSort] = useState(null);
  const [limit, setLimit] = useState(0);
  const [trees, setTrees] = useState(null);
  const [recent, setRecent] = useState([]);
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    getTreeStats().then(setTrees).catch(() => setTrees(null));
  }, []);

  useEffect(() => {
    getRecentGames().then(setRecent).catch(() => setRecent([]));
  }, [version]);

  const run = async () => {
    setBusy(true);
    try {
      setSort(await runSortingLab(limit));
    } finally {
      setBusy(false);
    }
  };

  const yes = (b) => (b ? 'yes' : 'no');

  return (
    <>
      <div className="panel">
        <h2>Sorting Lab</h2>
        <span className="ds-tag">7 sorts, same shuffled dictionary</span>
        <div className="lab-form">
          <select className="select" value={limit} onChange={(e) => setLimit(Number(e.target.value))}>
            <option value={0}>All words</option>
            <option value={100}>First 100</option>
            <option value={300}>First 300</option>
          </select>
          <button className="btn primary" onClick={run} disabled={busy}>
            {busy ? 'Sorting...' : 'Run'}
          </button>
        </div>
        {sort && (
          <table className="data">
            <thead>
              <tr>
                <th>Sort ({sort.inputSize})</th>
                <th>O()</th>
                <th className="num">Cmp</th>
                <th className="num">Moves</th>
                <th className="num">Time</th>
              </tr>
            </thead>
            <tbody>
              {sort.runs.map((r) => (
                <tr key={r.algorithm}>
                  <td>{r.algorithm}{!r.sortedCorrectly && ' (!)'}</td>
                  <td className="mono small">{r.complexity}</td>
                  <td className="num">{r.comparisons}</td>
                  <td className="num">{r.swaps}</td>
                  <td className="num">{formatTime(r.microseconds * 1000)}</td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>

      {trees && (
        <div className="panel">
          <h2>Dictionary trees</h2>
          <span className="ds-tag">binary tree properties, 2-3 tree shape</span>
          <table className="data">
            <tbody>
              <tr><td>AVL nodes / leaves / internal</td><td className="num">{trees.avl.nodes} / {trees.avl.leaves} / {trees.avl.internalNodes}</td></tr>
              <tr><td>AVL height (min possible)</td><td className="num">{trees.avl.height} ({trees.avl.minPossibleHeight})</td></tr>
              <tr><td>Full / complete / perfect</td><td className="num">{yes(trees.avl.full)} / {yes(trees.avl.complete)} / {yes(trees.avl.perfect)}</td></tr>
              <tr><td>2-3 tree keys / nodes</td><td className="num">{trees.btree.keys} / {trees.btree.nodes}</td></tr>
              <tr><td>2-3 tree height / valid</td><td className="num">{trees.btree.height} / {yes(trees.btree.valid)}</td></tr>
            </tbody>
          </table>
        </div>
      )}

      <div className="panel">
        <h2>Recent games</h2>
        <span className="ds-tag">ArrayQueue (circular, keeps last 10)</span>
        {recent.length === 0 ? (
          <p className="muted">No finished games yet.</p>
        ) : (
          <table className="data">
            <tbody>
              {recent.map((g) => (
                <tr key={g.gameId}>
                  <td className="mono">{g.secretWord}</td>
                  <td>{g.result}</td>
                  <td>{g.difficulty}</td>
                  <td className="num">{g.attempts} tries</td>
                  <td className="num">{g.score}</td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </>
  );
}
