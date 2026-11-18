/** @type {import('tailwindcss').Config} */
export default {
  darkMode: 'class',
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      fontFamily: {
        sans: ['"Fira Sans"', 'system-ui', '-apple-system', 'BlinkMacSystemFont', 'sans-serif'],
        mono: ['"Fira Code"', 'ui-monospace', 'monospace']
      },
      colors: {
        surface: {
          base: 'var(--surface-base)',
          raised: 'var(--surface-raised)',
          sunken: 'var(--surface-sunken)',
          overlay: 'var(--surface-overlay)'
        },
        border: {
          subtle: 'var(--border-subtle)',
          strong: 'var(--border-strong)'
        },
        text: {
          primary: 'var(--text-primary)',
          secondary: 'var(--text-secondary)',
          muted: 'var(--text-muted)'
        },
        accent: {
          DEFAULT: 'var(--accent-default)',
          hover: 'var(--accent-hover)',
          focusRing: 'var(--accent-focus-ring)'
        },
        status: {
          clear: {
            text: 'var(--status-clear-text)',
            bg: 'var(--status-clear-bg)',
            border: 'var(--status-clear-border)',
            chart: 'var(--status-clear-chart)'
          },
          buildingUp: {
            text: 'var(--status-building-text)',
            bg: 'var(--status-building-bg)',
            border: 'var(--status-building-border)',
            chart: 'var(--status-building-chart)'
          },
          danger: {
            text: 'var(--status-danger-text)',
            bg: 'var(--status-danger-bg)',
            border: 'var(--status-danger-border)',
            chart: 'var(--status-danger-chart)'
          },
          offline: {
            text: 'var(--status-offline-text)',
            bg: 'var(--status-offline-bg)',
            border: 'var(--status-offline-border)',
            chart: 'var(--status-offline-chart)'
          }
        }
      }
    },
  },
  plugins: [],
}
