import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

// The C++ backend listens on localhost:8080.
// Every request to /api is forwarded to it, so the frontend can call
// relative URLs (e.g. fetch('/api/game/new')) with no CORS setup in dev.
export default defineConfig({
  plugins: [react()],
  server: {
    port: 5173,
    proxy: {
      '/api': 'http://localhost:8080',
    },
  },
});
