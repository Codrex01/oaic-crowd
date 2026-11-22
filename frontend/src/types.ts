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

export interface HardwareActuatorState {
  systemMode: 'AI_VISION_PRIMARY' | 'HARDWARE_FAILOVER';
  cameraOnline: boolean;
  lastCameraPing: number;
  currentCount: number;
  radarEnergy: number;
  tofDistanceMm: number;
  ledState: { red: boolean; yellow: boolean; green: boolean };
  rgb: { r: number; g: number; b: number };
  ledPins: { red: number; yellow: number; green: number };
  servoAngle: number;
  servoStatus: string;
  buzzer: boolean;
  oled: {
    line1: string;
    line2: string;
    line3: string;
    line4: string;
  };
}

