import { useEffect, useState } from 'react';
import { getReplay, saveScore } from '../api/client.js';
import Tile from './Tile.jsx';

// Shown when a game ends: score, name entry (won games only) and a replay
// that steps forward / back through the server's DoublyLinkedList<GuessFeedback>.
export default function GameOverModal({ game, onClose, onSaved }) {
  const [replay, setReplay] = useState(null);
  const [step, setStep] = useState(0);
  const [name, setName] = useState('');
  const [saved, setSaved] = useState(null);
  const [error, setError] = useState('');

  useEffect(() => {
    getReplay(game.gameId)
      .then((r) => {
        setReplay(r);
        setStep(r.steps.length - 1);
      })
      .catch((e) => setError(e.message));
  }, [game.gameId]);

  const submitName = async (e) => {
    e.preventDefault();
    try {
      setSaved(await saveScore(game.gameId, name.trim() || 'Anonymous'));
      onSaved();
    } catch (err) {
      setError(err.message);
    }
  };

  const shown = replay ? replay.steps.slice(0, step + 1) : [];

  return (
    <div className="modal-backdrop" onClick={onClose}>
      <div className="modal panel" onClick={(e) => e.stopPropagation()}>
        <h2>{game.status === 'won' ? 'Solved!' : 'Out of attempts'}</h2>
        <p className="mono">
          Word: <strong>{game.secretWord}</strong> - score {game.score}
        </p>

        {game.status === 'won' && !saved && (
          <form className="lab-form" onSubmit={submitName}>
            <input
              className="select"
              value={name}
              maxLength={20}
              placeholder="your name"
              onChange={(e) => setName(e.target.value)}
            />
            <button className="btn primary" type="submit">
              Save score
            </button>
          </form>
        )}
        {saved && <p className="muted">Saved for {saved.name}.</p>}

        <h2>Replay</h2>
        <span className="ds-tag">DoublyLinkedList - step back and forward</span>
        {replay && (
          <>
            <div className="board replay-board">
              {shown.map((s) => (
                <div className="board-row" key={s.step}>
                  {[...s.guess].map((ch, i) => (
                    <Tile key={i} letter={ch} state={s.feedback[i]} />
                  ))}
                </div>
              ))}
            </div>
            <div className="lab-form">
              <button className="btn" onClick={() => setStep((x) => Math.max(0, x - 1))} disabled={step === 0}>
                &larr; prev
              </button>
              <span className="mono muted">
                step {step + 1} / {replay.steps.length}
              </span>
              <button
                className="btn"
                onClick={() => setStep((x) => Math.min(replay.steps.length - 1, x + 1))}
                disabled={step === replay.steps.length - 1}
              >
                next &rarr;
              </button>
            </div>
          </>
        )}
        {error && <p className="status-line error">{error}</p>}
        <button className="btn" onClick={onClose}>
          Close
        </button>
      </div>
    </div>
  );
}
