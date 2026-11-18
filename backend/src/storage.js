import fs from 'fs';
import path from 'path';
import { config } from './config.js';

class StorageManager {
  constructor() {
    this.zones = new Map();
    this.telemetryRingBuffers = new Map(); // zoneId -> Array of up to 1000 records
    this.alerts = [];                      // Historical alerts list
    this.commandQueues = new Map();        // deviceId -> Array of queued commands
    this.rateLimiter = new Map();          // deviceId -> { count, windowStart }

    this.initDefaultZones();
    this.ensureDataDir();
  }

  ensureDataDir() {
    try {
      if (!fs.existsSync(config.dataDir)) {
        fs.mkdirSync(config.dataDir, { recursive: true });
      }
    } catch (err) {
      console.warn('[STORAGE] Could not create data directory:', err.message);
    }
  }

  initDefaultZones() {
    this.zones.set('zone-entry-a', {
      zoneId: 'zone-entry-a',
      name: 'Main Temple Gate A',
      areaSqM: 24.0,
      corridorWidthM: 4.0,
      deviceId: 'esp32-radar-01',
      lastTelemetry: null
    });

    this.zones.set('zone-corridor-b', {
      zoneId: 'zone-corridor-b',
      name: 'North Pavilion Corridor',
      areaSqM: 32.0,
      corridorWidthM: 3.5,
      deviceId: 'esp32-radar-02',
      lastTelemetry: null
    });
  }

  checkRateLimit(deviceId) {
    const now = Date.now();
    const entry = this.rateLimiter.get(deviceId) || { count: 0, windowStart: now };

    if (now - entry.windowStart >= 1000) {
      entry.count = 1;
      entry.windowStart = now;
      this.rateLimiter.set(deviceId, entry);
      return true;
    }

    if (entry.count >= config.rateLimitReqPerSec) {
      return false; // Rate limit exceeded (more than 2 req/sec)
    }

    entry.count++;
    this.rateLimiter.set(deviceId, entry);
    return true;
  }

  addTelemetry(telemetry) {
    const { zoneId, state, score, deviceId } = telemetry;

    // Retrieve or register zone
    let zone = this.zones.get(zoneId);
    if (!zone) {
      zone = {
        zoneId,
        name: `Zone ${zoneId}`,
        areaSqM: 24.0,
        corridorWidthM: 4.0,
        deviceId: deviceId || 'unknown',
        lastTelemetry: null
      };
      this.zones.set(zoneId, zone);
    }

    const previousState = zone.lastTelemetry ? zone.lastTelemetry.state : 'GREEN';
    const serverTimestamp = Date.now();
    const record = { ...telemetry, serverTimestamp };

    // Update zone's latest telemetry pointer
    zone.lastTelemetry = record;

    // Check for state escalation to AMBER or RED to generate alerts
    if (previousState !== state && (state === 'AMBER' || state === 'RED')) {
      const alert = {
        id: `alert-${Date.now()}-${Math.floor(Math.random() * 1000)}`,
        timestamp: serverTimestamp,
        zoneId,
        zoneName: zone.name,
        fromState: previousState,
        toState: state,
        score,
        density: telemetry.density,
        avgSpeed: telemetry.avgSpeed,
        message: state === 'RED'
          ? `CRITICAL: Severe crowd pressure & stall detected in ${zone.name} (Risk: ${score})`
          : `WARNING: Crowd density buildup approaching threshold in ${zone.name} (Risk: ${score})`
      };
      this.alerts.unshift(alert);

      // Keep last 500 alerts in memory
      if (this.alerts.length > 500) {
        this.alerts.pop();
      }
    }

    // Add to circular buffer (last 1000 points)
    if (!this.telemetryRingBuffers.has(zoneId)) {
      this.telemetryRingBuffers.set(zoneId, []);
    }
    const buffer = this.telemetryRingBuffers.get(zoneId);
    buffer.push(record);
    if (buffer.length > config.maxPointsPerZone) {
      buffer.shift();
    }

    return record;
  }

  getZones() {
    return Array.from(this.zones.values()).map(z => ({
      ...z,
      currentStatus: z.lastTelemetry ? z.lastTelemetry.state : 'OFFLINE',
      score: z.lastTelemetry ? z.lastTelemetry.score : 0,
      density: z.lastTelemetry ? z.lastTelemetry.density : 0,
      avgSpeed: z.lastTelemetry ? z.lastTelemetry.avgSpeed : 0,
      lastSeenMsAgo: z.lastTelemetry ? (Date.now() - z.lastTelemetry.serverTimestamp) : null
    }));
  }

  getZoneHistory(zoneId, minutes = 10) {
    const buffer = this.telemetryRingBuffers.get(zoneId) || [];
    const cutoffTime = Date.now() - (minutes * 60 * 1000);
    return buffer.filter(point => point.serverTimestamp >= cutoffTime);
  }

  getAlerts(limit = 100) {
    return this.alerts.slice(0, limit);
  }

  queueCommand(deviceId, command, reason = '') {
    if (!this.commandQueues.has(deviceId)) {
      this.commandQueues.set(deviceId, []);
    }

    const cmdRecord = {
      id: `cmd-${Date.now()}-${Math.floor(Math.random() * 1000)}`,
      deviceId,
      command,
      reason,
      createdAt: Date.now(),
      expiresAt: Date.now() + config.commandExpiryMs
    };

    this.commandQueues.get(deviceId).push(cmdRecord);
    return cmdRecord;
  }

  getPendingCommand(deviceId) {
    const queue = this.commandQueues.get(deviceId);
    if (!queue || queue.length === 0) {
      return null;
    }

    const now = Date.now();
    // Discard expired commands (>30s)
    while (queue.length > 0 && queue[0].expiresAt < now) {
      console.log(`[STORAGE] Dropped expired command for ${deviceId}:`, queue.shift().command);
    }

    if (queue.length === 0) {
      return null;
    }

    // Return and consume oldest valid command
    return queue.shift();
  }

  updateZoneConfig(zoneId, updates) {
    const zone = this.zones.get(zoneId);
    if (!zone) return null;

    if (updates.name) zone.name = updates.name;
    if (updates.areaSqM) zone.areaSqM = updates.areaSqM;
    if (updates.corridorWidthM) zone.corridorWidthM = updates.corridorWidthM;

    return zone;
  }
}

export const storage = new StorageManager();

