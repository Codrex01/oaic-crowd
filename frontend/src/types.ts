export type CrowdState = 'GREEN' | 'AMBER' | 'RED' | 'OFFLINE';

export interface TelemetryPoint {
  deviceId: string;
  zoneId: string;
  timestamp: number;
  serverTimestamp: number;
  state: 'GREEN' | 'AMBER' | 'RED';
  score: number;
  density: number;
  avgSpeed: number;
  stopGoIndex: number;
  targetCount: number;
  entryCount: number;
  exitCount: number;
  activeAlerts: string[];
  firmwareVersion?: string;
}

export interface ZoneInfo {
  zoneId: string;
  name: string;
  areaSqM: number;
  corridorWidthM: number;
  deviceId: string;
  currentStatus: CrowdState;
  score: number;
  density: number;
  avgSpeed: number;
  lastSeenMsAgo: number | null;
  lastTelemetry: TelemetryPoint | null;
}

export interface AlertRecord {
  id: string;
  timestamp: number;
  zoneId: string;
  zoneName: string;
  fromState: string;
  toState: string;
  score: number;
  density?: number;
  avgSpeed?: number;
  message: string;
}

export type OperatorCommand = 'MANUAL_GATE_OPEN' | 'MANUAL_GATE_CLOSE' | 'SILENCE_BUZZER' | 'RESET_ZONE';

