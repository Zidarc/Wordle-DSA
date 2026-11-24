// Hints come from the server's Queue<int> of shuffled positions
// (GameSession::useHint dequeues until it finds an unsolved position).
export default function HintButton({ hints, hintsUsed, onHint, disabled }) {
  return (
    <div className="hint-box">
      <button className="btn" onClick={onHint} disabled={disabled}>
        Hint (-50)
      </button>
      <span className="muted mono">used {hintsUsed}</span>
      {hints.map((h) => (
        <span key={h.position} className="hint-chip mono">
          #{h.position + 1} = {h.letter}
        </span>
      ))}
    </div>
  );
}
