import { z } from 'zod';

export const TelemetrySchema = z.object({
  deviceId: z.string().min(1),
  zoneId: z.string().min(1),
  timestamp: z.coerce.number().int().positive(),
  state: z.enum(['GREEN', 'AMBER', 'RED']),
  score: z.coerce.number().min(0).max(100),
  density: z.coerce.number().min(0),
  avgSpeed: z.coerce.number().min(0),
  stopGoIndex: z.coerce.number().min(0),
  targetCount: z.coerce.number().int().min(0),
  entryCount: z.coerce.number().int().min(0),
  exitCount: z.coerce.number().int().min(0),
  activeAlerts: z.array(z.string()).default([]),
  firmwareVersion: z.string().default('1.0.0')
});

export const CommandSchema = z.object({
  deviceId: z.string().min(1),
  command: z.enum(['MANUAL_GATE_OPEN', 'MANUAL_GATE_CLOSE', 'SILENCE_BUZZER', 'RESET_ZONE']),
  reason: z.string().optional()
});

export const ZoneConfigSchema = z.object({
  zoneId: z.string().min(1),
  name: z.string().min(1),
  areaSqM: z.coerce.number().positive(),
  corridorWidthM: z.coerce.number().positive()
});

