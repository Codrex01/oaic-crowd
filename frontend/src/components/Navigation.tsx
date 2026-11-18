import React from 'react';

export interface NavTabItem {
  id: string;
  label: string;
  count?: number;
}

interface NavigationProps {
  activeTab: string;
  setActiveTab: (tabId: string) => void;
  alertCount: number;
}

export const Navigation: React.FC<NavigationProps> = ({
  activeTab,
  setActiveTab,
  alertCount
}) => {
  const navItems: NavTabItem[] = [
    { id: 'overview', label: 'Overview' },
    { id: 'detail', label: 'Zone details' },
    { id: 'alerts', label: 'Alerts', count: alertCount },
    { id: 'setup', label: 'Setup' },
    { id: 'about', label: 'About & safety' }
  ];

  return (
    <>
      {/* Desktop Left Sidebar Navigation */}
      <nav
        aria-label="Primary navigation"
        className="hidden md:flex flex-col w-52 shrink-0 border-r border-border-subtle bg-surface-raised p-3 gap-1 min-h-[calc(100vh-85px)]"
      >
        <span className="text-[11px] font-mono uppercase tracking-wider text-text-muted px-2 py-1.5">
          Navigation
        </span>
        {navItems.map(item => {
          const isActive = activeTab === item.id;
          return (
            <button
              key={item.id}
              onClick={() => setActiveTab(item.id)}
              className={`flex items-center justify-between px-3 py-2 text-xs font-medium rounded transition-colors text-left focus-visible:outline ${
                isActive
                  ? 'bg-surface-sunken text-text-primary border border-border-strong font-semibold'
                  : 'text-text-secondary hover:text-text-primary hover:bg-surface-sunken'
              }`}
              aria-current={isActive ? 'page' : undefined}
            >
              <span>{item.label}</span>
              {typeof item.count === 'number' && item.count > 0 && (
                <span className="ml-2 font-mono text-[10px] px-1.5 py-0.2 border border-border-subtle rounded bg-surface-base text-text-secondary">
                  {item.count}
                </span>
              )}
            </button>
          );
        })}
      </nav>

      {/* Mobile Top Navigation Bar (Normal document flow beneath header, zero bottom overlap) */}
      <nav
        aria-label="Mobile navigation"
        className="md:hidden border-b border-border-subtle bg-surface-raised flex items-center overflow-x-auto p-1.5 gap-1 shrink-0 scrollbar-none"
      >
        {navItems.map(item => {
          const isActive = activeTab === item.id;
          return (
            <button
              key={item.id}
              onClick={() => setActiveTab(item.id)}
              className={`flex items-center gap-1.5 px-3 py-1.5 rounded text-xs whitespace-nowrap transition-colors focus-visible:outline ${
                isActive
                  ? 'bg-surface-sunken text-text-primary border border-border-strong font-semibold'
                  : 'text-text-secondary hover:text-text-primary'
              }`}
              aria-current={isActive ? 'page' : undefined}
            >
              <span>{item.label}</span>
              {typeof item.count === 'number' && item.count > 0 && (
                <span className="font-mono text-[10px] px-1 py-0.2 border border-border-subtle rounded bg-surface-base text-text-secondary">
                  {item.count}
                </span>
              )}
            </button>
          );
        })}
      </nav>
    </>
  );
};

