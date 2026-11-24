// Difficulty -> attempts / multiplier, as in backend/include/Difficulty.h.
const LEVELS = [
  { value: 'easy', label: 'Easy', detail: '8 tries, x1.0' },
  { value: 'normal', label: 'Normal', detail: '6 tries, x1.5' },
  { value: 'hard', label: 'Hard', detail: '5 tries, x2.0, hints must be reused' },
];

export default function DifficultySelect({ value, onChange }) {
  return (
    <select className="select" value={value} onChange={(e) => onChange(e.target.value)} title="Difficulty">
      {LEVELS.map((l) => (
        <option key={l.value} value={l.value}>
          {l.label} - {l.detail}
        </option>
      ))}
    </select>
  );
}
