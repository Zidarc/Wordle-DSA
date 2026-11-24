// On-screen keyboard. The three rows (10, 9, 7 keys) come from the server's
// LetterTracker jagged array; each key is coloured by its best known state
// (array of 26, indexed by letter - 'A').
const ROWS = ['QWERTYUIOP', 'ASDFGHJKL', 'ZXCVBNM'];

export default function Keyboard({ letterStatus, onLetter, onEnter, onBackspace, disabled }) {
  return (
    <div className="keyboard">
      {ROWS.map((row, r) => (
        <div className="key-row" key={r}>
          {r === 2 && (
            <button className="key key-wide" onClick={onEnter} disabled={disabled}>
              Enter
            </button>
          )}
          {[...row].map((ch) => (
            <button
              key={ch}
              className={`key key-${letterStatus[ch] ?? 'unused'}`}
              onClick={() => onLetter(ch)}
              disabled={disabled}
            >
              {ch}
            </button>
          ))}
          {r === 2 && (
            <button className="key key-wide" onClick={onBackspace} disabled={disabled} aria-label="Backspace">
              ⌫
            </button>
          )}
        </div>
      ))}
    </div>
  );
}
