import { useState } from 'react';

// Tabbed container for the DSA panels on the right.
// tabs: [{ id, label, render: () => JSX }]
export default function SidePanel({ tabs }) {
  const [active, setActive] = useState(tabs[0].id);
  const current = tabs.find((t) => t.id === active) ?? tabs[0];
  return (
    <aside className="side">
      <nav className="tabs">
        {tabs.map((t) => (
          <button
            key={t.id}
            className={`tab${t.id === current.id ? ' active' : ''}`}
            onClick={() => setActive(t.id)}
          >
            {t.label}
          </button>
        ))}
      </nav>
      {current.render()}
    </aside>
  );
}
