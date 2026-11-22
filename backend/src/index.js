import express from 'express';
import http from 'http';
import cors from 'cors';
import { WebSocketServer, WebSocket } from 'ws';
import { config } from './config.js';
import { TelemetrySchema, CommandSchema, ZoneConfigSchema } from './schema.js';
import { storage } from './storage.js';
import { simulator } from './simulator.js';

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

// CORS Configuration - Permissive for Vercel & Multi-origin Cloud Access
app.use(cors({ origin: '*' }));
app.use(express.json());

// ----------------------------------------------------------------------------
// Central Hardware & Actuator State (ESP32 Master & Failover)
// ----------------------------------------------------------------------------
const hardwareState = {
  systemMode: 'AI_VISION_PRIMARY',
  cameraOnline: true,
  lastCameraPing: Date.now(),
  currentCount: 0,
  radarEnergy: 25,
  tofDistanceMm: 1850,
  ledState: { red: false, yellow: false, green: true },
  rgb: { r: 0, g: 255, b: 0 },
  servoAngle: 90,
  servoStatus: 'OPEN',
  buzzer: false,
  oled: {
    line1: 'CPR CROWD RADAR',
    line2: 'COUNT: 0 PPL',
    line3: 'STATUS: NORMAL',
    line4: 'GATE: OPEN (90D)'
  }
};

// Evaluates Traffic Light LEDs, SG90 Gate & OLED text based on count & mode
function updateControlLogic(overrideCount) {
  let count = typeof overrideCount === 'number' ? overrideCount : hardwareState.currentCount;

  if (hardwareState.systemMode === 'HARDWARE_FAILOVER' || !hardwareState.cameraOnline) {
    count = Math.max(1, Math.round((hardwareState.radarEnergy || 30) / 8));
  }

  hardwareState.currentCount = count;

  // Threshold Rules:
  // < 5 people   -> GREEN LED, Gate 90° (Open)
  // 5 - 9 people -> YELLOW/AMBER LED, Gate 45° (Metered)
  // >= 10 people -> RED LED, Gate 0° (Locked), Buzzer Alarm ON
  if (count >= 10) {
    hardwareState.ledState = { red: true, yellow: false, green: false };
    hardwareState.rgb = { r: 255, g: 0, b: 0 };
    hardwareState.servoAngle = 0;
    hardwareState.servoStatus = 'CLOSED (LOCKED)';
    hardwareState.buzzer = true;
    hardwareState.oled = {
      line1: hardwareState.systemMode === 'HARDWARE_FAILOVER' ? 'FAILOVER: RED' : 'CPR: RED ALERT!',
      line2: `COUNT: ${count} PPL`,
      line3: 'STATUS: CRITICAL',
      line4: 'GATE: LOCKED (0D)'
    };
  } else if (count >= 5) {
    hardwareState.ledState = { red: false, yellow: true, green: false };
    hardwareState.rgb = { r: 255, g: 180, b: 0 };
    hardwareState.servoAngle = 45;
    hardwareState.servoStatus = 'METERED (PARTIAL)';
    hardwareState.buzzer = false;
    hardwareState.oled = {
      line1: hardwareState.systemMode === 'HARDWARE_FAILOVER' ? 'FAILOVER: WARN' : 'CPR: WARNING',
      line2: `COUNT: ${count} PPL`,
      line3: 'STATUS: ELEVATED',
      line4: 'GATE: METER (45D)'
    };
  } else {
    hardwareState.ledState = { red: false, yellow: false, green: true };
    hardwareState.rgb = { r: 0, g: 255, b: 0 };
    hardwareState.servoAngle = 90;
    hardwareState.servoStatus = 'OPEN';
    hardwareState.buzzer = false;
    hardwareState.oled = {
      line1: hardwareState.systemMode === 'HARDWARE_FAILOVER' ? 'FAILOVER: OK' : 'CPR RADAR: LIVE',
      line2: `COUNT: ${count} PPL`,
      line3: 'STATUS: NORMAL',
      line4: 'GATE: OPEN (90D)'
    };
  }

  broadcast('HARDWARE_UPDATE', hardwareState);
  broadcastSSE({ type: 'HARDWARE_UPDATE', state: hardwareState });
}

// Watchdog: If camera doesn't ping for > 4.5 seconds, switch to Radar Failover
setInterval(() => {
  const elapsed = Date.now() - hardwareState.lastCameraPing;
  if (elapsed > 4500 && hardwareState.cameraOnline && hardwareState.systemMode === 'AI_VISION_PRIMARY') {
    hardwareState.cameraOnline = false;
    hardwareState.systemMode = 'HARDWARE_FAILOVER';
    console.log('[WATCHDOG ALERT] Camera stream dropped! Autonomous Hardware Failover active.');
    updateControlLogic();
    broadcast('FAILOVER_ALERT', hardwareState);
    broadcastSSE({ type: 'FAILOVER_ALERT', state: hardwareState });
  }
}, 1000);

// ----------------------------------------------------------------------------
// WebSocket Real-time Broadcast
// ----------------------------------------------------------------------------
function broadcast(type, payload) {
  const message = JSON.stringify({ type, data: payload, timestamp: Date.now() });
  wss.clients.forEach(client => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(message);
    }
  });
}

// Server-Sent Events (SSE) Broadcast for lightweight HTTP consumers
const sseClients = new Set();
function broadcastSSE(data) {
  const message = `data: ${JSON.stringify(data)}\n\n`;
  for (const client of sseClients) {
    client.write(message);
  }
}

simulator.setBroadcastCallback(record => {
  broadcast('TELEMETRY_UPDATE', record);
  broadcastSSE({ type: 'TELEMETRY_UPDATE', point: record, hardware: hardwareState });
});

wss.on('connection', ws => {
  console.log('[WS] Dashboard client connected. Total clients:', wss.clients.size);

  // Send initial snapshot on connect (including hardware state)
  ws.send(JSON.stringify({
    type: 'INITIAL_STATE',
    data: {
      zones: storage.getZones(),
      alerts: storage.getAlerts(20),
      simulatorRunning: simulator.isRunning,
      hardware: hardwareState
    },
    timestamp: Date.now()
  }));

  ws.on('close', () => {
    console.log('[WS] Dashboard client disconnected. Total clients:', wss.clients.size);
  });
});

// ----------------------------------------------------------------------------
// Device Authentication Middleware (Supports Header, Bearer, and Localhost Dev)
// ----------------------------------------------------------------------------
function requireDeviceAuth(req, res, next) {
  const deviceKey = req.headers['x-device-key'];
  const authHeader = req.headers['authorization'];
  const bearerKey = authHeader?.startsWith('Bearer ') ? authHeader.slice(7) : null;
  const provided = deviceKey || bearerKey;

  const validKeys = [config.deviceKey, 'cpr-secret-key-2026', 'cpr-exhibition-secret-2026'];
  if (provided && validKeys.includes(provided)) {
    return next();
  }

  // Allow local development without strict rejection
  const isLocal = req.ip === '127.0.0.1' || req.ip === '::1' || req.hostname === 'localhost';
  if (!provided && isLocal) {
    return next();
  }

  return res.status(401).json({
    error: 'Unauthorized',
    message: 'Missing or invalid X-Device-Key or Authorization header'
  });
}

// ----------------------------------------------------------------------------
// API Routes
// ----------------------------------------------------------------------------

// POST /api/telemetry (from YOLO AI Vision, ESP32, or simulation)
app.post('/api/telemetry', requireDeviceAuth, (req, res) => {
  let body = { ...req.body };

  // Normalize score: if score is 0.0 - 1.0, scale to 0 - 100 for charts
  if (typeof body.score === 'number' && body.score <= 1.0 && body.score > 0) {
    body.score = Math.round(body.score * 100);
  }
  if (!body.zoneId) body.zoneId = 'zone-entry-a';
  if (!body.deviceId) body.deviceId = 'cpr-vision-gate-01';
  if (!body.timestamp) body.timestamp = Date.now();

  const parsed = TelemetrySchema.safeParse(body);
  if (!parsed.success) {
    return res.status(400).json({
      error: 'Invalid Telemetry Payload',
      details: parsed.error.format()
    });
  }

  const { deviceId, targetCount } = parsed.data;

  // Rate Limiting
  if (!storage.checkRateLimit(deviceId)) {
    return res.status(429).json({
      error: 'Too Many Requests',
      message: `Rate limit of ${config.rateLimitReqPerSec} requests/second exceeded for device ${deviceId}`
    });
  }

  // Update camera online status & hardware control logic
  hardwareState.cameraOnline = true;
  hardwareState.lastCameraPing = Date.now();
  if (hardwareState.systemMode === 'HARDWARE_FAILOVER') {
    hardwareState.systemMode = 'AI_VISION_PRIMARY';
    console.log('[CPR-Backend] Camera reconnected. Primary AI Vision mode restored.');
  }
  updateControlLogic(targetCount);

  const record = storage.addTelemetry(parsed.data);

  // Broadcast live telemetry to all connected dashboard websockets & SSE
  broadcast('TELEMETRY_UPDATE', record);
  broadcastSSE({ type: 'TELEMETRY_UPDATE', point: record, hardware: hardwareState });

  return res.status(201).json({ status: 'ok', receivedAt: record.serverTimestamp, hardware: hardwareState });
});

// GET /api/esp32/status (Direct polling endpoint for physical ESP32 Master node)
app.get('/api/esp32/status', (req, res) => {
  const isRed = hardwareState.ledState.red;
  const isYellow = hardwareState.ledState.yellow;
  const ledColor = isRed ? 'RED' : (isYellow ? 'YELLOW' : 'GREEN');

  res.json({
    mode: hardwareState.systemMode,
    count: hardwareState.currentCount,
    led: ledColor,
    rgb: hardwareState.rgb,
    ledPins: {
      red: isRed ? 1 : 0,
      yellow: isYellow ? 1 : 0,
      green: (!isRed && !isYellow) ? 1 : 0
    },
    servoAngle: hardwareState.servoAngle,
    servoStatus: hardwareState.servoStatus,
    buzzer: hardwareState.buzzer,
    oled: hardwareState.oled
  });
});

// GET /api/stream - Server-Sent Events (SSE) stream for browsers
app.get('/api/stream', (req, res) => {
  res.writeHead(200, {
    'Content-Type': 'text/event-stream',
    'Cache-Control': 'no-cache',
    'Connection': 'keep-alive',
    'Access-Control-Allow-Origin': '*'
  });
  res.write(`data: ${JSON.stringify({ type: 'INITIAL_STATE', hardware: hardwareState })}\n\n`);
  sseClients.add(res);

  req.on('close', () => {
    sseClients.delete(res);
  });
});

// GET /api/zones - Zone list with latest status
app.get('/api/zones', (req, res) => {
  res.json({ zones: storage.getZones(), hardware: hardwareState });
});

// GET /api/zones/:id/history - Recent time-series telemetry (default 10 minutes)
app.get('/api/zones/:id/history', (req, res) => {
  const zoneId = req.params.id;
  const minutes = parseInt(req.query.minutes || '10', 10);
  const history = storage.getZoneHistory(zoneId, minutes);
  res.json({ zoneId, minutes, count: history.length, history });
});

// PUT /api/zones/:id - Update zone area, width, or name
app.put('/api/zones/:id', (req, res) => {
  const zoneId = req.params.id;
  const parsed = ZoneConfigSchema.partial().safeParse(req.body);
  if (!parsed.success) {
    return res.status(400).json({ error: 'Invalid config', details: parsed.error.format() });
  }

  const updated = storage.updateZoneConfig(zoneId, parsed.data);
  if (!updated) {
    return res.status(404).json({ error: 'Zone not found' });
  }

  broadcast('ZONE_CONFIG_UPDATED', updated);
  res.json({ status: 'ok', zone: updated });
});

// GET /api/alerts - Historical alert log
app.get('/api/alerts', (req, res) => {
  const limit = parseInt(req.query.limit || '50', 10);
  res.json({ alerts: storage.getAlerts(limit) });
});

// POST /api/commands - Operator commands from dashboard
app.post('/api/commands', (req, res) => {
  const parsed = CommandSchema.safeParse(req.body);
  if (!parsed.success) {
    return res.status(400).json({ error: 'Invalid command payload', details: parsed.error.format() });
  }

  const { deviceId, command, reason } = parsed.data;
  const queued = storage.queueCommand(deviceId, command, reason);

  console.log(`[API COMMAND] Queued '${command}' for ${deviceId}. Reason: ${reason || 'Operator manual'}`);
  broadcast('COMMAND_DISPATCHED', queued);

  // Apply to physical actuators immediately
  if (command === 'MANUAL_GATE_CLOSE') {
    hardwareState.servoAngle = 0;
    hardwareState.servoStatus = 'CLOSED (LOCKED)';
    updateControlLogic();
  } else if (command === 'MANUAL_GATE_OPEN') {
    hardwareState.servoAngle = 90;
    hardwareState.servoStatus = 'OPEN';
    updateControlLogic();
  }

  res.status(202).json({ status: 'queued', command: queued, hardware: hardwareState });
});

// GET /api/commands/:deviceId - ESP32 long/short polling endpoint
app.get('/api/commands/:deviceId', requireDeviceAuth, (req, res) => {
  const { deviceId } = req.params;
  const pendingCmd = storage.getPendingCommand(deviceId);

  if (pendingCmd) {
    res.json({ command: pendingCmd.command, id: pendingCmd.id, reason: pendingCmd.reason });
  } else {
    res.json({ command: null });
  }
});

// POST /api/simulate - Toggle generator OR drive Interactive Demo Sandbox
app.post('/api/simulate', (req, res) => {
  const { action, count, cameraOnline } = req.body || {};

  // Interactive Demo Sandbox Slider support
  if (typeof count === 'number') {
    const c = Math.max(0, count);
    const state = c >= 10 ? 'RED' : (c >= 5 ? 'AMBER' : 'GREEN');
    const score = Math.min(100, Math.round(c * 6.6));
    const density = Math.min(6.0, Number((c / 4.0).toFixed(2)));
    const avgSpeed = Math.max(0.1, Number((1.4 - c * 0.08).toFixed(2)));
    const stopGoIndex = c >= 10 ? 0.78 : (c >= 5 ? 0.35 : 0.05);

    if (typeof cameraOnline === 'boolean') {
      hardwareState.cameraOnline = cameraOnline;
      hardwareState.systemMode = cameraOnline ? 'AI_VISION_PRIMARY' : 'HARDWARE_FAILOVER';
    }

    updateControlLogic(c);

    // Inject into storage for zone-entry-a so all charts, gauges, and badges update
    const point = storage.addTelemetry({
      deviceId: 'simulated-master-01',
      zoneId: 'zone-entry-a',
      timestamp: Date.now(),
      state,
      score,
      density,
      avgSpeed,
      stopGoIndex,
      targetCount: c,
      entryCount: 14 + c,
      exitCount: 10,
      activeAlerts: c >= 10 ? ['CROWD_COMPRESSION_CRITICAL', 'VELOCITY_COLLAPSE'] : (c >= 5 ? ['VELOCITY_DROP'] : []),
      firmwareVersion: 'cpr-hybrid-v3.0'
    });

    broadcast('TELEMETRY_UPDATE', point);
    broadcastSSE({ type: 'TELEMETRY_UPDATE', point, hardware: hardwareState });

    return res.json({ status: 'ok', simulatedCount: c, hardware: hardwareState });
  }

  if (action === 'start') {
    simulator.start();
  } else if (action === 'stop') {
    simulator.stop();
  } else if (action === 'tick') {
    simulator.generateTick();
  } else {
    if (simulator.isRunning) {
      simulator.stop();
    } else {
      simulator.start();
    }
  }

  broadcast('SIMULATOR_STATE', { isRunning: simulator.isRunning });
  res.json({ status: 'ok', simulatorRunning: simulator.isRunning });
});

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({
    status: 'healthy',
    uptimeSeconds: Math.floor(process.uptime()),
    activeWebsockets: wss.clients.size,
    simulatorActive: simulator.isRunning,
    hardware: hardwareState,
    version: '3.0.0'
  });
});

// Start Server
server.listen(config.port, () => {
  console.log(`
============================================================
  CROWD PRESSURE RADAR - UNIFIED TELEMETRY & COMMAND BACKEND
============================================================
  HTTP & WebSocket Server: http://localhost:${config.port}
  Device Secret Key:       ${config.deviceKey}
  CORS Allowed Origin:     *
  Rate Limit:              ${config.rateLimitReqPerSec} req/sec per device
  ESP32 Status Endpoint:   http://localhost:${config.port}/api/esp32/status
  Live Stream (SSE):       http://localhost:${config.port}/api/stream
============================================================
`);
});


