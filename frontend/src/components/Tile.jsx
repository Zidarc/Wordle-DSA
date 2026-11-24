// One letter cell. `state` is a LetterState name from the server:
// "correct" | "present" | "absent" | "unused" (GUIDE: Part 3, two-pass evaluate).
export default function Tile({ letter = '', state = 'unused', active = false, delay = 0 }) {
  const classes = ['tile', `tile-${state}`];
  if (letter && state === 'unused') classes.push('tile-filled');
  if (active) classes.push('tile-active');
  return (
    <div className={classes.join(' ')} style={{ animationDelay: `${delay}ms` }}>
      {letter}
    </div>
  );
}
