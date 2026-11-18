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

// Middleware
app.use(cors({ origin: config.corsOrigin }));
app.use(express.json());

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

simulator.setBroadcastCallback(record => {
  broadcast('TELEMETRY_UPDATE', record);
});

wss.on('connection', ws => {
  console.log('[WS] Dashboard client connected. Total clients:', wss.clients.size);

  // Send initial snapshot on connect
  ws.send(JSON.stringify({
    type: 'INITIAL_STATE',
    data: {
      zones: storage.getZones(),
      alerts: storage.getAlerts(20),
      simulatorRunning: simulator.isRunning
    },
    timestamp: Date.now()
  }));

  ws.on('close', () => {
    console.log('[WS] Dashboard client disconnected. Total clients:', wss.clients.size);
  });
});

// ----------------------------------------------------------------------------
// Device Authentication Middleware
// ----------------------------------------------------------------------------
function requireDeviceAuth(req, res, next) {
  const deviceKey = req.headers['x-device-key'];
  if (!deviceKey || deviceKey !== config.deviceKey) {
    return res.status(401).json({
      error: 'Unauthorized',
      message: 'Missing or invalid X-Device-Key header'
    });
  }
  next();
}

// ----------------------------------------------------------------------------
// API Routes
// ----------------------------------------------------------------------------

// POST /api/telemetry (from ESP32 or simulation)
app.post('/api/telemetry', requireDeviceAuth, (req, res) => {
  const parsed = TelemetrySchema.safeParse(req.body);
  if (!parsed.success) {
    return res.status(400).json({
      error: 'Invalid Telemetry Payload',
      details: parsed.error.format()
    });
  }

  const { deviceId } = parsed.data;

  // Rate Limiting: 2 requests / sec per device
  if (!storage.checkRateLimit(deviceId)) {
    return res.status(429).json({
      error: 'Too Many Requests',
      message: `Rate limit of ${config.rateLimitReqPerSec} requests/second exceeded for device ${deviceId}`
    });
  }

  const record = storage.addTelemetry(parsed.data);

  // Broadcast live telemetry to all connected dashboard websockets
  broadcast('TELEMETRY_UPDATE', record);

  return res.status(201).json({ status: 'ok', receivedAt: record.serverTimestamp });
});

// GET /api/zones - Zone list with latest status
app.get('/api/zones', (req, res) => {
  res.json({ zones: storage.getZones() });
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

  res.status(202).json({ status: 'queued', command: queued });
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

// POST /api/simulate - Toggle or control mock telemetry generator
app.post('/api/simulate', (req, res) => {
  const action = req.body?.action || 'toggle';

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
    version: '1.0.0'
  });
});

// Start Server
server.listen(config.port, () => {
  console.log(`
============================================================
  CROWD PRESSURE RADAR - TELEMETRY & COMMAND BACKEND
============================================================
  HTTP & WebSocket Server: http://localhost:${config.port}
  Device Secret Key:       ${config.deviceKey}
  CORS Allowed Origin:     ${config.corsOrigin}
  Rate Limit:              ${config.rateLimitReqPerSec} req/sec per device
============================================================
`);
});

