import React from 'react';
import { ZoneInfo } from '../types';
import { StateBadge } from './StateBadge';

interface ZoneCardProps {
  zone: ZoneInfo;
  onSelect: (zoneId: string) => void;
}

export const ZoneCard: React.FC<ZoneCardProps> = ({ zone, onSelect }) => {
  const status = zone.currentStatus || 'OFFLINE';

  const formatAge = (ms: number | null | undefined): string => {
    if (ms === null || ms === undefined || isNaN(ms)) return 'Waiting for sensor';
    const sec = Math.floor(ms / 1000);
    if (sec < 2) return 'Just now';
    if (sec < 60) return `Updated ${sec}s ago`;
    const min = Math.floor(sec / 60);
    return `Updated ${min}m ago`;
  };

  const scorePercentage = Math.min(100, Math.max(0, zone.score));

  // Bar color based strictly on status
  const barColor =
    status === 'RED'
      ? 'var(--status-danger-chart)'
      : status === 'AMBER'
      ? 'var(--status-building-chart)'
      : status === 'GREEN'
      ? 'var(--status-clear-chart)'
      : 'var(--status-offline-chart)';

  return (
    <button
      onClick={() => onSelect(zone.zoneId)}
      className="flat-panel p-4 text-left transition-colors hover:border-border-strong focus-visible:outline w-full flex flex-col justify-between"
      aria-label={`View details for ${zone.name}. Current state: ${status}. Risk score: ${zone.score.toFixed(0)}`}
    >
      {/* Top Header: Zone Name & Quad-coded State Badge */}
      <div className="flex items-start justify-between gap-2 mb-3">
        <div>
          <h3 className="text-sm font-semibold text-text-primary">
            {zone.name}
          </h3>
          <span className="font-mono text-[11px] text-text-muted">
            {zone.zoneId}
          </span>
        </div>
        <StateBadge state={status} size="sm" />
      </div>

      {/* Risk Score: Number + 0-100 Linear Bar */}
      <div className="mb-3">
        <div className="flex items-baseline justify-between mb-1">
          <span className="text-xs text-text-secondary">
            Crowd risk score (0–100)
          </span>
          <span className="font-mono text-sm font-bold text-text-primary">
            {zone.score.toFixed(1)} <span className="text-[10px] text-text-muted font-normal">/ 100</span>
          </span>
        </div>
        <div className="w-full bg-surface-sunken border border-border-subtle rounded-full h-2 overflow-hidden">
          <div
            className="h-full transition-all duration-300"
            style={{
              width: `${scorePercentage}%`,
              backgroundColor: barColor
            }}
            role="progressbar"
            aria-valuenow={scorePercentage}
            aria-valuemin={0}
            aria-valuemax={100}
          />
        </div>
      </div>

      {/* Metric Readouts: Density & Average Speed */}
      <div className="grid grid-cols-2 gap-2 pt-2 border-t border-border-subtle text-xs">
        <div>
          <span className="text-text-muted block text-[11px]">Crowd density</span>
          <span className="font-mono font-semibold text-text-primary">
            {zone.density.toFixed(2)}{' '}
            <span className="text-[10px] text-text-secondary font-normal">people/m²</span>
          </span>
        </div>
        <div>
          <span className="text-text-muted block text-[11px]">Average speed</span>
          <span className="font-mono font-semibold text-text-primary">
            {zone.avgSpeed.toFixed(2)}{' '}
            <span className="text-[10px] text-text-secondary font-normal">m/s</span>
          </span>
        </div>
      </div>

      {/* Footer: Updated Xs ago */}
      <div className="mt-3 pt-2 border-t border-border-subtle text-[11px] font-mono text-text-muted flex items-center justify-between">
        <span>{formatAge(zone.lastSeenMsAgo)}</span>
        <span className="text-accent underline text-[11px] font-sans">Open zone &rarr;</span>
      </div>
    </button>
  );
};
