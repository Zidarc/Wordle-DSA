import { useEffect, useState } from 'react';
import { getDsaStats, lookupWord } from '../api/client.js';
import LookupTable from './LookupTable.jsx';

// DSA Insights: how each of the six lookups handled the last word typed,
// a "lookup lab" to try any word, and dictionary structure stats.
// TIP: numbers come from Dictionary::lookupAll (timed with kTimingRepeats).
export default function DsaInsights({ lookup, method, version }) {
  const [stats, setStats] = useState(null);
  const [word, setWord] = useState('');
  const [lab, setLab] = useState(null);
  const [labError, setLabError] = useState('');

  useEffect(() => {
    getDsaStats().then(setStats).catch(() => setStats(null));
  }, [version]);

  const tryWord = async (e) => {
    e.preventDefault();
    try {
      setLab(await lookupWord(word, method));
      setLabError('');
    } catch (err) {
      setLab(null);
      setLabError(err.message);
    }
  };

  return (
    <div className="panel">
      <h2>DSA Insights</h2>
      <span className="ds-tag">6 lookups on the same word: comparisons, time, Big-O</span>

      {lookup ? (
        <>
          <p className="mono">
            {lookup.word}: {lookup.found ? 'in the list' : 'not in the list'}
          </p>
          <LookupTable rows={lookup.methods} />
        </>
      ) : (
        <p className="muted">Make a guess to see how each method finds it.</p>
      )}

      <form className="lab-form" onSubmit={tryWord}>
        <input
          className="select mono"
          value={word}
          maxLength={5}
          placeholder="any word"
          onChange={(e) => setWord(e.target.value.toUpperCase())}
        />
        <button className="btn" type="submit" disabled={word.length !== 5}>
          Look up
        </button>
      </form>
      {labError && <p className="status-line error">{labError}</p>}
      {lab && (
        <>
          <p className="mono">
            {lab.word}: {lab.found ? 'found' : 'not found'} (time = average of {lab.timingRepeats} runs)
          </p>
          <LookupTable rows={lab.methods} />
        </>
      )}

      {stats && (
        <table className="data stats">
          <tbody>
            <tr><td>Words</td><td className="num">{stats.wordCount}</td></tr>
            <tr><td>AVL height / leaves</td><td className="num">{stats.avlHeight} / {stats.avlLeaves}</td></tr>
            <tr><td>2-3 tree height / nodes</td><td className="num">{stats.btreeHeight} / {stats.btreeNodes}</td></tr>
            <tr><td>Hash buckets (used)</td><td className="num">{stats.hashBuckets} ({stats.hashUsedBuckets})</td></tr>
            <tr><td>Load factor / longest chain</td><td className="num">{stats.hashLoadFactor.toFixed(2)} / {stats.hashLongestChain}</td></tr>
            <tr><td>Rehashes</td><td className="num">{stats.hashRehashCount}</td></tr>
          </tbody>
        </table>
      )}
    </div>
  );
}
