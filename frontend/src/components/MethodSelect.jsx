// Which lookup decides "is this word in the list?" (sent as `method` with each
// guess). Server side: Dictionary::methodFromString + LookupStats::get.
export const METHODS = [
  { key: 'linear', label: 'Linear search - O(n)' },
  { key: 'binary', label: 'Binary search - O(log n)' },
  { key: 'interpolation', label: 'Interpolation - O(log log n)' },
  { key: 'avl', label: 'AVL tree - O(log n)' },
  { key: 'btree', label: '2-3 tree - O(log n)' },
  { key: 'hash', label: 'Hash table - O(1)' },
];

export default function MethodSelect({ value, onChange }) {
  return (
    <select className="select" value={value} onChange={(e) => onChange(e.target.value)} title="Lookup method">
      {METHODS.map((m) => (
        <option key={m.key} value={m.key}>
          {m.label}
        </option>
      ))}
    </select>
  );
}
