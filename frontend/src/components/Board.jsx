import Tile from './Tile.jsx';

// The grid mirrors the server's Board class (a dynamic 2D array):
// rows = maxAttempts (8 / 6 / 5 by difficulty), cols = 5.
//
// rows:    finished guesses [{ guess, feedback: [state x5] }]
// current: letters typed in the active row
// shake:   true for a moment after a rejected guess
export default function Board({ rows, current, maxAttempts, wordLength = 5, shake = false }) {
  const grid = [];
  for (let r = 0; r < maxAttempts; r++) {
    const cells = [];
    const done = rows[r];
    const isActive = r === rows.length;
    for (let c = 0; c < wordLength; c++) {
      if (done) {
        cells.push(<Tile key={c} letter={done.guess[c]} state={done.feedback[c]} delay={c * 90} />);
      } else if (isActive) {
        cells.push(<Tile key={c} letter={current[c] ?? ''} active={c === current.length} />);
      } else {
        cells.push(<Tile key={c} />);
      }
    }
    grid.push(
      <div key={r} className={`board-row${isActive && shake ? ' shake' : ''}`}>
        {cells}
      </div>
    );
  }
  return <div className="board">{grid}</div>;
}
