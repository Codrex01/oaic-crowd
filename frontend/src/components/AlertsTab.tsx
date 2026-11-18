import React, { useState } from 'react';
import { Download, Search } from 'lucide-react';
import { AlertRecord } from '../types';
import { StateBadge } from './StateBadge';
import { formatAlertSentence } from '../lib/formatter';

interface AlertsTabProps {
  alerts: AlertRecord[];
}

export const AlertsTab: React.FC<AlertsTabProps> = ({ alerts }) => {
  const [searchTerm, setSearchTerm] = useState('');
  const [filterState, setFilterState] = useState<'ALL' | 'RED' | 'AMBER'>('ALL');
  const [filterZone, setFilterZone] = useState<string>('ALL');

  // Unique list of zones in alerts
  const uniqueZones = Array.from(new Set(alerts.map(a => a.zoneName)));

  const filteredAlerts = alerts.filter(alert => {
    const matchesSearch =
      alert.zoneName.toLowerCase().includes(searchTerm.toLowerCase()) ||
      alert.zoneId.toLowerCase().includes(searchTerm.toLowerCase()) ||
      alert.message.toLowerCase().includes(searchTerm.toLowerCase());

    const matchesSeverity = filterState === 'ALL' ? true : alert.toState === filterState;
    const matchesZone = filterZone === 'ALL' ? true : alert.zoneName === filterZone;

    return matchesSearch && matchesSeverity && matchesZone;
  });

  const exportCsv = () => {
    if (alerts.length === 0) return;

    const headers = ['ID', 'Timestamp_ISO', 'Zone_ID', 'Zone_Name', 'From_State', 'To_State', 'Risk_Score', 'Message'];
    const rows = filteredAlerts.map(a => [
      `"${a.id}"`,
      `"${new Date(a.timestamp).toISOString()}"`,
      `"${a.zoneId}"`,
      `"${a.zoneName}"`,
      `"${a.fromState}"`,
      `"${a.toState}"`,
      a.score,
      `"${a.message.replace(/"/g, '""')}"`
    ]);

    const csvContent = [headers.join(','), ...rows.map(r => r.join(','))].join('\n');
    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.setAttribute('href', url);
    link.setAttribute('download', `crowd_radar_alerts_${Date.now()}.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  };

  return (
    <div className="space-y-4">
      {/* 1. Header & Plain Controls */}
      <div className="flat-panel p-4 flex flex-col md:flex-row items-stretch md:items-center justify-between gap-3">
        {/* Search & Filters */}
        <div className="flex flex-wrap items-center gap-2 flex-1">
          <div className="relative min-w-[200px]">
            <Search className="w-3.5 h-3.5 text-text-muted absolute left-2.5 top-2.5" aria-hidden="true" />
            <input
              type="text"
              placeholder="Filter by keyword..."
              value={searchTerm}
              onChange={e => setSearchTerm(e.target.value)}
              className="w-full pl-8 pr-2.5 py-1.5 text-xs bg-surface-sunken border border-border-strong rounded text-text-primary placeholder:text-text-muted focus-visible:outline"
            />
          </div>

          {/* Filter by State */}
          <div className="flex items-center border border-border-strong rounded bg-surface-sunken p-0.5 text-xs">
            <button
              onClick={() => setFilterState('ALL')}
              className={`px-2.5 py-1 rounded text-xs font-medium transition-colors ${
                filterState === 'ALL' ? 'bg-surface-raised text-text-primary font-semibold' : 'text-text-secondary hover:text-text-primary'
              }`}
            >
              All ({alerts.length})
            </button>
            <button
              onClick={() => setFilterState('RED')}
              className={`px-2.5 py-1 rounded text-xs font-medium transition-colors ${
                filterState === 'RED' ? 'bg-status-danger-bg text-status-danger-text font-semibold' : 'text-text-secondary hover:text-status-danger-text'
              }`}
            >
              Danger ({alerts.filter(a => a.toState === 'RED').length})
            </button>
            <button
              onClick={() => setFilterState('AMBER')}
              className={`px-2.5 py-1 rounded text-xs font-medium transition-colors ${
                filterState === 'AMBER' ? 'bg-status-building-bg text-status-building-text font-semibold' : 'text-text-secondary hover:text-status-building-text'
              }`}
            >
              Building up ({alerts.filter(a => a.toState === 'AMBER').length})
            </button>
          </div>

          {/* Filter by Zone */}
          {uniqueZones.length > 1 && (
            <select
              value={filterZone}
              onChange={e => setFilterZone(e.target.value)}
              className="bg-surface-raised border border-border-strong text-text-primary text-xs rounded px-2.5 py-1.5 focus-visible:outline"
              style={{ backgroundColor: 'var(--surface-raised)', color: 'var(--text-primary)' }}
            >
              <option value="ALL">All zones</option>
              {uniqueZones.map(z => (
                <option key={z} value={z}>
                  {z}
                </option>
              ))}
            </select>
          )}
        </div>

        {/* CSV Export Button */}
        <button
          onClick={exportCsv}
          disabled={alerts.length === 0}
          className="flex items-center justify-center gap-1.5 px-3 py-1.5 border border-border-strong bg-surface-sunken hover:bg-surface-raised rounded text-xs font-medium text-text-primary transition-colors focus-visible:outline disabled:opacity-40"
        >
          <Download className="w-3.5 h-3.5 text-text-secondary" aria-hidden="true" />
          <span>Export CSV log</span>
        </button>
      </div>

      {/* 2. Readable Plain-Sentence List */}
      <div className="space-y-2">
        {filteredAlerts.length === 0 ? (
          <div className="flat-panel p-8 text-center text-xs text-text-muted">
            No alerts recorded. All monitored queue zones are operating within normal limits.
          </div>
        ) : (
          filteredAlerts.map(alert => {
            const date = new Date(alert.timestamp);
            const timeStr = date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
            const sentence = formatAlertSentence(
              alert.zoneName,
              alert.fromState,
              alert.toState,
              timeStr,
              alert.score
            );

            return (
              <div
                key={alert.id}
                className="flat-panel p-3.5 flex flex-col sm:flex-row sm:items-center justify-between gap-2.5"
              >
                <div className="flex items-start sm:items-center gap-3">
                  <StateBadge state={alert.toState as any} size="sm" />
                  <div>
                    <p className="text-xs text-text-primary font-medium leading-relaxed">
                      {sentence}
                    </p>
                    <span className="text-[11px] font-mono text-text-muted block mt-0.5">
                      {date.toLocaleDateString()} {timeStr} • Zone ID: {alert.zoneId}
                    </span>
                  </div>
                </div>

                <div className="text-right shrink-0">
                  <span className="font-mono text-xs font-bold text-text-primary">
                    Risk {alert.score.toFixed(0)}{' '}
                    <span className="text-[10px] text-text-muted font-normal">/ 100</span>
                  </span>
                </div>
              </div>
            );
          })
        )}
      </div>
    </div>
  );
};
