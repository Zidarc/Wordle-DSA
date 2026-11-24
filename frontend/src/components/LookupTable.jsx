// Table of the six lookup methods for one word: comparisons, measured time,
// and Big-O. Rows come from the server's lookupMethodsJson() (Server.cpp).

// ns -> "734 ns" / "32.3 µs" / "1.20 ms"
export function formatTime(ns) {
  if (ns >= 1e6) return `${(ns / 1e6).toFixed(2)} ms`;
  if (ns >= 1e3) return `${(ns / 1e3).toFixed(1)} µs`;
  return `${ns} ns`;
}

export default function LookupTable({ rows }) {
  const maxNs = Math.max(1, ...rows.map((r) => r.nanoseconds));
  return (
    <table className="data lookup">
      <thead>
        <tr>
          <th>Method</th>
          <th className="num">Cmp</th>
          <th className="num">Time</th>
          <th>Time O()</th>
          <th>Extra space</th>
        </tr>
      </thead>
      <tbody>
        {rows.map((r) => (
          <tr key={r.key} className={r.used ? 'used' : ''}>
            <td>
              {r.name} {r.used && <span className="used-tag">used</span>}
              <div className="bar" style={{ width: `${(r.nanoseconds / maxNs) * 100}%` }} />
            </td>
            <td className="num">{r.comparisons}</td>
            <td className="num">{formatTime(r.nanoseconds)}</td>
            <td className="mono" title={`best ${r.timeBest}, worst ${r.timeWorst}`}>
              {r.timeAvg}
            </td>
            <td className="mono small" title={`structure: ${r.spaceStructure}`}>
              {r.spaceExtra}
            </td>
          </tr>
        ))}
      </tbody>
    </table>
  );
}
