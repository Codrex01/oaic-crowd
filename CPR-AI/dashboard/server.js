/**
 * CPR (Crowd Pressure Radar) - Multi-Page Telemetry Hub & Exhibition Server
 * Pure Node.js (Zero external dependencies)
 * 
 * Routes:
 * - GET  /              : Live Operational Exhibition Dashboard
 * - GET  /demo          : Interactive Hardware Architecture & Component Demo
 * - POST /api/telemetry : Ingests telemetry from YOLO AI and ESP32 nodes
 * - GET  /api/telemetry : Returns current combined system state
 * - GET  /api/stream    : Real-time Server-Sent Events (SSE) stream for both pages
 * - GET  /api/esp32/status : Endpoint for ESP32 to poll LED states, OLED text, and servo angle
 * - POST /api/failover  : Simulates/Toggles camera failure and hardware fallback
 */

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 5000;

// Central System State
const state = {
  // Operational Mode: 'AI_VISION_PRIMARY' or 'HARDWARE_FAILOVER'
  systemMode: 'AI_VISION_PRIMARY',
  cameraOnline: true,
  lastCameraPing: Date.now(),

  // Live Vision Metrics (YOLO + ByteTrack)
  vision: {
    deviceId: 'cpr-vision-gate-01',
    zoneId: 'zone-corridor-a',
    timestamp: Date.now(),
    state: 'GREEN',
    score: 0.15,
    density: 0.12,
    avgSpeed: 48.5,
    stopGoIndex: 0.0,
    targetCount: 2,
    entryCount: 14,
    exitCount: 11,
    activeAlerts: [],
    firmwareVersion: 'cpr-ai-v2.4-cuda'
  },

  // Physical ESP32 Sensor Hardware State (Breadboard 1 & 2)
  hardware: {
    esp32MasterOnline: true,
    esp32SlaveOnline: true,
    lastSeen: Date.now(),
    
    // Breadboard 1 (Right): Gate, 24GHz Radar, ToF, Master LoRa
    ld2410RadarPresence: false,
    ld2410Energy: 22,
    tofDistanceMm: 1350,
    loraMasterRssi: -68,
    
    // Breadboard 2 (Left): IR Beam, Slave LoRa
    irBeamBroken: false,
    irBeamTriggerCount: 14,
    loraSlaveRssi: -72,

    // Traffic Light LEDs (Red, Yellow, Green)
    ledState: {
      red: false,
      yellow: false,
      green: true
    },

    // SG90 Servo Gate Actuator
    servoAngle: 90, // 90 = Open, 0 = Close/Lock
    servoStatus: 'OPEN',

    // Physical 0.96" I2C OLED Screen Content
    oledScreen: {
      line1: 'CPR CROWD RADAR',
      line2: 'COUNT: 2 PPL',
      line3: 'STATUS: NORMAL',
      line4: 'GATE: OPEN (90 DEG)'
    },

    buzzerActive: false
  },

  // System Event Logs
  eventLogs: [
    { timestamp: new Date().toLocaleTimeString(), type: 'INFO', msg: 'CPR Multi-Component Telemetry Hub online on port 5000.' }
  ]
};

// SSE Client Connections
const sseClients = new Set();

function broadcastSSE(data) {
  const message = `data: ${JSON.stringify(data)}\n\n`;
  for (const client of sseClients) {
    client.write(message);
  }
}

function logEvent(type, msg) {
  const entry = { timestamp: new Date().toLocaleTimeString(), type, msg };
  state.eventLogs.unshift(entry);
  if (state.eventLogs.length > 40) state.eventLogs.pop();
}

// Evaluates LED Traffic Lights, SG90 Gate, and OLED Display based on Active Mode
function updateControlLogic() {
  // Determine effective count based on active mode (YOLO vs Hardware Fallback)
  let count = 0;
  let activeSource = 'CAMERA AI';

  if (state.systemMode === 'AI_VISION_PRIMARY' && state.cameraOnline) {
    count = state.vision.targetCount;
    activeSource = 'CAMERA AI (PRIMARY)';
  } else {
    // Hardware Fallback: Estimate from 24GHz Radar Energy + IR beam break counts
    count = Math.max(1, Math.round(state.hardware.ld2410Energy / 8));
    activeSource = 'ESP32 HARDWARE (FAILOVER)';
  }

  // Threshold Rules:
  // < 5 people   -> GREEN LED
  // 5 - 9 people -> YELLOW LED
  // >= 10 people -> RED LED (+ Servo Gate Closed + Buzzer)
  if (count >= 10) {
    state.vision.state = 'RED';
    state.hardware.ledState = { red: true, yellow: false, green: false };
    state.hardware.servoAngle = 0;
    state.hardware.servoStatus = 'CLOSED (LOCKED)';
    state.hardware.buzzerActive = true;
    state.hardware.oledScreen = {
      line1: 'CPR: RED ALERT!',
      line2: `COUNT: ${count} PPL`,
      line3: 'STATUS: CRITICAL',
      line4: 'GATE: LOCKED (0D)'
    };
  } else if (count >= 5) {
    state.vision.state = 'AMBER';
    state.hardware.ledState = { red: false, yellow: true, green: false };
    state.hardware.servoAngle = 45;
    state.hardware.servoStatus = 'METERED (PARTIAL)';
    state.hardware.buzzerActive = false;
    state.hardware.oledScreen = {
      line1: 'CPR: WARNING',
      line2: `COUNT: ${count} PPL`,
      line3: 'STATUS: ELEVATED',
      line4: 'GATE: METER (45D)'
    };
  } else {
    state.vision.state = 'GREEN';
    state.hardware.ledState = { red: false, yellow: false, green: true };
    state.hardware.servoAngle = 90;
    state.hardware.servoStatus = 'OPEN';
    state.hardware.buzzerActive = false;
    state.hardware.oledScreen = {
      line1: 'CPR RADAR: LIVE',
      line2: `COUNT: ${count} PPL`,
      line3: 'STATUS: NORMAL',
      line4: 'GATE: OPEN (90D)'
    };
  }
}

// Watchdog timer: If camera doesn't ping for > 4.5 seconds, trigger automatic hardware failover!
setInterval(() => {
  const elapsed = Date.now() - state.lastCameraPing;
  if (elapsed > 4500 && state.cameraOnline && state.systemMode === 'AI_VISION_PRIMARY') {
    state.cameraOnline = false;
    state.systemMode = 'HARDWARE_FAILOVER';
    logEvent('ALERT', 'CAMERA STREAM OFFLINE — Automatic failover triggered to ESP32 Radar & ToF sensors!');
    updateControlLogic();
    broadcastSSE({ type: 'FAILOVER_ALERT', state });
  }
}, 1000);

const server = http.createServer((req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization, x-source');

  if (req.method === 'OPTIONS') {
    res.writeHead(204);
    return res.end();
  }

  const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
  const pathname = parsedUrl.pathname;

  // 1. Ingest Telemetry (POST /api/telemetry)
  if (req.method === 'POST' && pathname === '/api/telemetry') {
    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      try {
        const payload = JSON.parse(body);
        const headerSource = req.headers['x-source'] || '';
        const payloadSource = payload.source || '';
        const isVision = headerSource.includes('vision') || payloadSource.includes('vision') || payloadSource === 'computer_vision';
        const isEsp32 = headerSource.includes('esp32') || payloadSource.includes('esp32');

        if (isVision || (!isEsp32 && typeof payload.targetCount === 'number')) {
          state.vision = { ...state.vision, ...payload, timestamp: Date.now() };
          state.cameraOnline = true;
          state.lastCameraPing = Date.now();
          if (state.systemMode === 'HARDWARE_FAILOVER') {
            state.systemMode = 'AI_VISION_PRIMARY';
            logEvent('INFO', 'Camera stream restored. Primary AI Vision mode resumed.');
          }
        } else if (isEsp32) {
          state.hardware = { ...state.hardware, ...payload, lastSeen: Date.now() };
        }

        updateControlLogic();
        broadcastSSE({ type: 'TELEMETRY_UPDATE', state });

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ status: 'ok', mode: state.systemMode }));
      } catch (err) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Invalid JSON' }));
      }
    });
    return;
  }

  // 2. Query Full State (GET /api/telemetry)
  if (req.method === 'GET' && pathname === '/api/telemetry') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    return res.end(JSON.stringify(state));
  }

  // 3. ESP32 Direct Status Query (GET /api/esp32/status)
  // ESP32 calls this via HTTP or Serial to know which LED to turn ON and servo position
  if (req.method === 'GET' && pathname === '/api/esp32/status') {
    const isRed = state.hardware.ledState.red;
    const isYellow = state.hardware.ledState.yellow;
    const ledColor = isRed ? 'RED' : (isYellow ? 'YELLOW' : 'GREEN');
    
    // RGB 3-in-1 LED values (0-255)
    // Red: 255,0,0 | Yellow: 255,180,0 (Red + Green mix) | Green: 0,255,0
    const rgb = isRed 
      ? { r: 255, g: 0, b: 0 } 
      : (isYellow ? { r: 255, g: 180, b: 0 } : { r: 0, g: 255, b: 0 });

    const espResponse = {
      mode: state.systemMode,
      count: state.vision.targetCount,
      led: ledColor,
      rgb: rgb,
      ledPins: {
        red: isRed ? 1 : 0,
        yellow: isYellow ? 1 : 0,
        green: (!isRed && !isYellow) ? 1 : 0
      },
      servoAngle: state.hardware.servoAngle,
      servoStatus: state.hardware.servoStatus,
      buzzer: state.hardware.buzzerActive,
      oled: state.hardware.oledScreen
    };
    res.writeHead(200, { 'Content-Type': 'application/json' });
    return res.end(JSON.stringify(espResponse));
  }

  // 4. Toggle / Simulate Failover (POST /api/failover)
  if (req.method === 'POST' && pathname === '/api/failover') {
    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      try {
        const data = body ? JSON.parse(body) : {};
        if (typeof data.cameraOnline === 'boolean') {
          state.cameraOnline = data.cameraOnline;
        } else {
          state.cameraOnline = !state.cameraOnline;
        }

        state.systemMode = state.cameraOnline ? 'AI_VISION_PRIMARY' : 'HARDWARE_FAILOVER';
        logEvent('STATUS', `System operational mode toggled to: ${state.systemMode}`);
        updateControlLogic();
        broadcastSSE({ type: 'MODE_CHANGE', state });

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ status: 'ok', state }));
      } catch (e) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: e.message }));
      }
    });
    return;
  }

  // 4b. Interactive Demo Simulation Override (POST /api/simulate)
  if (req.method === 'POST' && pathname === '/api/simulate') {
    let body = '';
    req.on('data', chunk => { body += chunk; });
    req.on('end', () => {
      try {
        const payload = JSON.parse(body);
        if (typeof payload.count === 'number') {
          const c = Math.max(0, payload.count);
          state.vision.targetCount = c;
          state.vision.score = Math.min(1.0, c / 15.0);
          state.vision.density = Math.min(1.0, c / 20.0);
          state.vision.avgSpeed = Math.max(5.0, 60.0 - c * 3.2);
          state.vision.stopGoIndex = c >= 10 ? 0.78 : (c >= 5 ? 0.35 : 0.05);
          state.hardware.ld2410Energy = Math.min(100, c * 7 + 12);
          state.hardware.ld2410RadarPresence = c > 0;
          state.hardware.tofDistanceMm = Math.max(280, 2000 - c * 95);
          state.lastCameraPing = Date.now();
        }
        if (typeof payload.cameraOnline === 'boolean') {
          state.cameraOnline = payload.cameraOnline;
          state.systemMode = payload.cameraOnline ? 'AI_VISION_PRIMARY' : 'HARDWARE_FAILOVER';
          logEvent('SIMULATION', `Mode forced to ${state.systemMode} via Interactive Demo`);
        }
        updateControlLogic();
        broadcastSSE({ type: 'SIMULATION_UPDATE', state });

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ status: 'ok', state }));
      } catch (e) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: e.message }));
      }
    });
    return;
  }

  // 5. Server-Sent Events (GET /api/stream)
  if (req.method === 'GET' && pathname === '/api/stream') {
    res.writeHead(200, {
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      'Connection': 'keep-alive'
    });
    res.write(`data: ${JSON.stringify({ type: 'INITIAL_STATE', state })}\n\n`);
    sseClients.add(res);

    req.on('close', () => {
      sseClients.delete(res);
    });
    return;
  }

  // 6. Static File Serving
  let targetFile = 'index.html';
  if (pathname === '/demo') {
    targetFile = 'demo.html';
  } else if (pathname !== '/') {
    targetFile = pathname.replace(/^\//, '');
  }

  const filePath = path.join(__dirname, 'public', targetFile);
  fs.readFile(filePath, (err, content) => {
    if (err) {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      return res.end('404 Page Not Found');
    }
    const ext = path.extname(filePath);
    let contentType = 'text/html';
    if (ext === '.js') contentType = 'text/javascript';
    if (ext === '.css') contentType = 'text/css';
    if (ext === '.json') contentType = 'application/json';
    if (ext === '.png') contentType = 'image/png';
    if (ext === '.jpg' || ext === '.jpeg') contentType = 'image/jpeg';

    res.writeHead(200, { 'Content-Type': contentType });
    res.end(content);
  });
});

server.listen(PORT, () => {
  console.log('='.repeat(65));
  console.log(`  CPR (Crowd Pressure Radar) Telemetry Hub & Multi-Page Server`);
  console.log(`  - Live Monitor Dashboard:   http://localhost:${PORT}/`);
  console.log(`  - Interactive Demo Sandbox: http://localhost:${PORT}/demo`);
  console.log(`  - ESP32 Direct Status API:  http://localhost:${PORT}/api/esp32/status`);
  console.log(`  - Telemetry Ingest:         http://localhost:${PORT}/api/telemetry`);
  console.log('='.repeat(65));
});
