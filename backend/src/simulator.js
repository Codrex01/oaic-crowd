import { storage } from './storage.js';

class TelemetrySimulator {
  constructor() {
    this.timer = null;
    this.step = 0;
    this.isRunning = false;
    this.broadcastCallback = null;
  }

  setBroadcastCallback(cb) {
    this.broadcastCallback = cb;
  }

  start() {
    if (this.isRunning) return;
    this.isRunning = true;
    console.log('[SIMULATOR] Synthetic crowd telemetry engine started.');

    this.timer = setInterval(() => {
      this.generateTick();
    }, 1000);
  }

  stop() {
    if (!this.isRunning) return;
    clearInterval(this.timer);
    this.timer = null;
    this.isRunning = false;
    console.log('[SIMULATOR] Synthetic crowd telemetry engine paused.');
  }

  generateTick() {
    this.step = (this.step + 1) % 60; // 60-second scenario loop

    // Scenario timeline:
    // 0 - 15s: Normal Green flow (Score 15-28, Speed 1.2-1.4, Density ~0.15)
    // 16 - 28s: Amber buildup (Score 52-65, Speed 0.6-0.8, Density ~0.45)
    // 29 - 42s: Red stall crush (Score 82-94, Speed 0.06-0.12, High stop-go, Density ~0.85)
    // 43 - 59s: Recovery de-escalation
    let state = 'GREEN';
    let score = 20.0;
    let density = 0.15;
    let avgSpeed = 1.35;
    let stopGoIndex = 0.12;
    let targetCount = 2;
    let clusterPairs = 0;

    if (this.step >= 16 && this.step < 29) {
      state = 'AMBER';
      score = 54.0 + (this.step - 16) * 0.8;
      density = 0.38 + (this.step - 16) * 0.02;
      avgSpeed = 0.72 - (this.step - 16) * 0.02;
      stopGoIndex = 0.32;
      targetCount = 3;
      clusterPairs = 1;
    } else if (this.step >= 29 && this.step < 43) {
      state = 'RED';
      score = 82.0 + Math.sin(this.step) * 6.0;
      density = 0.75 + Math.random() * 0.1;
      avgSpeed = 0.08 + Math.random() * 0.05;
      stopGoIndex = 0.58 + Math.random() * 0.15;
      targetCount = 3;
      clusterPairs = 2;
    } else if (this.step >= 43) {
      state = (this.step < 50) ? 'AMBER' : 'GREEN';
      score = 65.0 - (this.step - 43) * 2.5;
      density = 0.30 - (this.step - 43) * 0.01;
      avgSpeed = 0.95 + (this.step - 43) * 0.02;
      stopGoIndex = 0.22;
      targetCount = 2;
      clusterPairs = 0;
    }

    const payloadA = {
      deviceId: 'esp32-radar-01',
      zoneId: 'zone-entry-a',
      timestamp: Date.now(),
      state,
      score: parseFloat(score.toFixed(1)),
      density: parseFloat(density.toFixed(2)),
      avgSpeed: parseFloat(avgSpeed.toFixed(2)),
      stopGoIndex: parseFloat(stopGoIndex.toFixed(2)),
      targetCount,
      entryCount: 142 + Math.floor(this.step / 3),
      exitCount: 130 + Math.floor(this.step / 4),
      activeAlerts: state === 'RED' ? ['CRITICAL_STALL_DETECTED'] : (state === 'AMBER' ? ['ELEVATED_DENSITY'] : []),
      firmwareVersion: '1.0.0-sim'
    };

    const recordA = storage.addTelemetry(payloadA);
    if (this.broadcastCallback) {
      this.broadcastCallback(recordA);
    }

    // Also simulate secondary zone (North Pavilion Corridor) with calm green flow
    const payloadB = {
      deviceId: 'esp32-radar-02',
      zoneId: 'zone-corridor-b',
      timestamp: Date.now(),
      state: 'GREEN',
      score: parseFloat((18.0 + Math.sin(this.step * 0.3) * 4.0).toFixed(1)),
      density: 0.18,
      avgSpeed: 1.25,
      stopGoIndex: 0.08,
      targetCount: 1,
      entryCount: 88 + Math.floor(this.step / 5),
      exitCount: 86 + Math.floor(this.step / 5),
      activeAlerts: [],
      firmwareVersion: '1.0.0-sim'
    };

    const recordB = storage.addTelemetry(payloadB);
    if (this.broadcastCallback) {
      this.broadcastCallback(recordB);
    }
  }
}

export const simulator = new TelemetrySimulator();

