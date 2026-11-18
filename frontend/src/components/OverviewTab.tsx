import React, { useState, useEffect } from 'react';
import {
  ResponsiveContainer,
  LineChart,
  Line,
  XAxis,
  YAxis,
  Tooltip,
  CartesianGrid,
  ReferenceLine
} from 'recharts';
import { ZoneInfo, CrowdState, TelemetryPoint } from '../types';
import { ZoneCard } from './ZoneCard';
import { fetchZoneHistory } from '../lib/api';
import { formatSystemActions } from '../lib/formatter';

interface OverviewTabProps {
  zones: ZoneInfo[];
  worstState: CrowdState;
  onSelectZone: (zoneId: string) => void;
}

export const OverviewTab: React.FC<OverviewTabProps> = ({
  zones,
  worstState,
  onSelectZone
}) => {
  const [history, setHistory] = useState<TelemetryPoint[]>([]);

  // Sort zones so worst status and highest risk score are first
  const sortedZones = React.useMemo(() => {
    const rank: Record<string, number> = { RED: 3, AMBER: 2, GREEN: 1, OFFLINE: 0 };
    return [...zones].sort((a, b) => {
      const rankDiff = (rank[b.currentStatus || 'OFFLINE'] || 0) - (rank[a.currentStatus || 'OFFLINE'] || 0);
      if (rankDiff !== 0) return rankDiff;
      return b.score - a.score;
    });
  }, [zones]);

  const worstZone = sortedZones[0] || null;
  const otherZones = sortedZones.slice(1);
  const primaryZoneId = worstZone?.zoneId || 'zone-entry-a';

  // Load 10-minute historical telemetry for the primary / worst zone
  useEffect(() => {
    let isMounted = true;
    async function loadData() {
      try {
        const data = await fetchZoneHistory(primaryZoneId, 10);
        if (isMounted) setHistory(data);
      } catch (err) {
        console.warn('Could not load history for overview:', err);
      }
    }
    loadData();
    const interval = setInterval(loadData, 3000);
    return () => {
      isMounted = false;
      clearInterval(interval);
    };
  }, [primaryZoneId]);

  // Format history for Recharts
  const chartData = history.map(item => {
    const timeDate = new Date(item.serverTimestamp || item.timestamp);
    return {
      time: timeDate.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' }),
      score: item.score
    };
  });

  const systemActions = formatSystemActions(
    worstState,
    worstZone?.score || 0,
    worstState === 'RED',
    worstState === 'RED'
  );

  return (
    <div className="space-y-6">
      {/* 1. Top Section: Row of Zone Tiles, with Worst Zone Shown First & Visually Separated */}
      <div>
        <div className="flex items-center justify-between mb-2">
          <h2 className="text-sm font-semibold uppercase tracking-wider text-text-secondary">
            Monitored Corridor Zones ({zones.length})
          </h2>
          <span className="text-xs text-text-muted">
            Select a zone to inspect radar and controls
          </span>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          {worstZone && (
            <div className="relative border-2 border-border-strong rounded p-1 bg-surface-raised flex flex-col">
              <div className="text-[10px] font-mono font-bold uppercase tracking-wider text-text-secondary px-2 pt-1 pb-1 flex items-center justify-between">
                <span>Priority Zone</span>
                <span className="text-[10px] text-text-muted">Highest crowd risk</span>
              </div>
              <ZoneCard zone={worstZone} onSelect={onSelectZone} />
            </div>
          )}

          {otherZones.map(zone => (
            <div key={zone.zoneId} className="flex flex-col">
              <div className="text-[10px] font-mono uppercase tracking-wider text-text-muted px-2 pt-1 pb-1">
                Monitored Zone
              </div>
              <ZoneCard zone={zone} onSelect={onSelectZone} />
            </div>
          ))}
        </div>
      </div>

      {/* 2. Middle Section: Large 10-Minute Risk Score Chart */}
      <div className="flat-panel p-5">
        <div className="flex flex-col sm:flex-row sm:items-baseline justify-between gap-1 mb-3">
          <div>
            <h3 className="text-base font-semibold text-text-primary">
              How has crowd risk changed over the last 10 minutes?
            </h3>
            <p className="text-xs text-text-secondary mt-0.5">
              Crowd risk score (0–100) plotted against Amber (50) and Red (75) safety thresholds.
            </p>
          </div>
          <div className="text-xs font-mono text-text-secondary">
            Zone: <span className="font-semibold text-text-primary">{worstZone?.name || primaryZoneId}</span>
          </div>
        </div>

        <div className="h-64 w-full">
          {chartData.length === 0 ? (
            <div className="h-full flex items-center justify-center text-xs font-mono text-text-muted">
              Waiting for telemetry data...
            </div>
          ) : (
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={chartData} margin={{ top: 12, right: 120, left: -20, bottom: 0 }}>
                <CartesianGrid strokeDasharray="3 3" stroke="var(--border-subtle)" />
                <XAxis
                  dataKey="time"
                  stroke="var(--text-muted)"
                  tick={{ fontSize: 11, fill: 'var(--text-secondary)' }}
                />
                <YAxis
                  domain={[0, 100]}
                  stroke="var(--text-muted)"
                  tick={{ fontSize: 11, fill: 'var(--text-secondary)' }}
                />
                <Tooltip
                  contentStyle={{
                    backgroundColor: 'var(--surface-raised)',
                    borderColor: 'var(--border-strong)',
                    borderRadius: '4px',
                    color: 'var(--text-primary)',
                    fontSize: '12px'
                  }}
                  formatter={(val: number) => [`${val.toFixed(1)} / 100`, 'Crowd risk score']}
                  labelStyle={{ color: 'var(--text-secondary)', fontSize: '11px' }}
                />
                <ReferenceLine
                  y={50}
                  stroke="var(--status-building-chart)"
                  strokeDasharray="4 4"
                  label={{ value: 'Amber (50)', fill: 'var(--status-building-chart)', fontSize: 11, position: 'right' }}
                />
                <ReferenceLine
                  y={75}
                  stroke="var(--status-danger-chart)"
                  strokeDasharray="4 4"
                  label={{ value: 'Red (75)', fill: 'var(--status-danger-chart)', fontSize: 11, position: 'right' }}
                />
                <Line
                  type="monotone"
                  dataKey="score"
                  name="Crowd risk score"
                  stroke="var(--accent-default)"
                  strokeWidth={2}
                  dot={false}
                  isAnimationActive={false}
                />
              </LineChart>
            </ResponsiveContainer>
          )}
        </div>
      </div>

      {/* 3. Bottom Section: What the system is doing now */}
      <div className="flat-panel p-5">
        <h3 className="text-base font-semibold text-text-primary mb-1">
          What the system is doing now
        </h3>
        <p className="text-xs text-text-secondary mb-3">
          Automatic physical actuator actions executed by sensor node state machine logic.
        </p>

        <ul className="space-y-2 text-xs">
          {systemActions.map((action, idx) => (
            <li key={idx} className="flex items-start gap-2 text-text-primary">
              <span className="text-text-muted select-none">•</span>
              <span>{action}</span>
            </li>
          ))}
        </ul>
      </div>
    </div>
  );
};
