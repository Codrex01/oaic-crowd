import { CrowdState } from '../types';

export interface StateDesignProps {
  label: string;
  badgeBg: string;
  badgeBorder: string;
  badgeText: string;
  indicatorColor: string;
  shape: 'circle' | 'triangle' | 'octagon' | 'square';
  iconName: 'Check' | 'AlertTriangle' | 'AlertOctagon' | 'WifiOff';
}

export const STATE_DESIGN: Record<CrowdState, StateDesignProps> = {
  GREEN: {
    label: 'Clear',
    badgeBg: 'bg-status-clear-bg',
    badgeBorder: 'border-status-clear-border',
    badgeText: 'text-status-clear-text',
    indicatorColor: '#22C55E',
    shape: 'circle',
    iconName: 'Check'
  },
  AMBER: {
    label: 'Building up',
    badgeBg: 'bg-status-building-bg',
    badgeBorder: 'border-status-building-border',
    badgeText: 'text-status-building-text',
    indicatorColor: '#F59E0B',
    shape: 'triangle',
    iconName: 'AlertTriangle'
  },
  RED: {
    label: 'Danger, barrier closed',
    badgeBg: 'bg-status-danger-bg',
    badgeBorder: 'border-status-danger-border',
    badgeText: 'text-status-danger-text',
    indicatorColor: '#EF4444',
    shape: 'octagon',
    iconName: 'AlertOctagon'
  },
  OFFLINE: {
    label: 'Offline / Disconnected',
    badgeBg: 'bg-status-offline-bg',
    badgeBorder: 'border-status-offline-border',
    badgeText: 'text-status-offline-text',
    indicatorColor: '#6B7280',
    shape: 'square',
    iconName: 'WifiOff'
  }
};

/**
 * Pure calculation replicating the firmware composite risk formula
 */
export function calculateFrontendRiskScore(
  density: number,
  avgSpeedMps: number,
  stopGoStdDev: number,
  clusterPairs: number
): number {
  // Density: 4.0 ppl/m² = 100%
  const sDensity = Math.min(100, (density / 4.0) * 100);

  // Speed: <= 0.15 m/s = 100% risk; >= 1.3 m/s = 0% risk
  let sLowSpeed = 0;
  if (avgSpeedMps <= 0.15) {
    sLowSpeed = 100;
  } else if (avgSpeedMps < 1.3) {
    sLowSpeed = ((1.3 - avgSpeedMps) / (1.3 - 0.15)) * 100;
    sLowSpeed = Math.min(100, Math.max(0, sLowSpeed));
  }

  // Stop-and-Go std-dev: 0.6 m/s = 100%
  const sStopGo = Math.min(100, (stopGoStdDev / 0.6) * 100);

  // Clustering: 50% per pair
  const sClustering = Math.min(100, clusterPairs * 50);

  // Weighted composite (0.4, 0.3, 0.2, 0.1)
  const composite = (0.4 * sDensity) + (0.3 * sLowSpeed) + (0.2 * sStopGo) + (0.1 * sClustering);
  return parseFloat(Math.min(100, Math.max(0, composite)).toFixed(1));
}
