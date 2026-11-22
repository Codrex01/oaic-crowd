/**
 * Render Free-Tier Keep-Alive Engine
 * Prevents Render services from spinning down / cold-sleeping after 15 minutes of inactivity.
 * Runs an automated self-ping loop every 10 minutes (600,000 ms).
 */

class KeepAliveEngine {
  constructor() {
    this.intervalMs = parseInt(process.env.KEEP_ALIVE_INTERVAL_MS || '600000', 10); // 10 minutes default
    this.timer = null;
    this.stats = {
      enabled: true,
      targetUrl: null,
      intervalMinutes: Math.round(this.intervalMs / 60000),
      totalPings: 0,
      successfulPings: 0,
      failedPings: 0,
      lastPingAt: null,
      lastPingStatus: 'INITIALIZING',
      lastLatencyMs: 0
    };
  }

  getTargetUrl(port = 3000) {
    // 1. Render automatically injects RENDER_EXTERNAL_URL (e.g. https://cpr-backend.onrender.com)
    // 2. Or explicit SELF_PING_URL or BACKEND_URL
    // 3. Fallback to local HTTP port
    const envUrl = process.env.RENDER_EXTERNAL_URL || process.env.SELF_PING_URL || process.env.BACKEND_URL;
    if (envUrl) {
      return envUrl.replace(/\/+$/, '');
    }
    return `http://localhost:${port}`;
  }

  async ping(port = 3000) {
    const baseUrl = this.getTargetUrl(port);
    const healthUrl = `${baseUrl}/health`;
    this.stats.targetUrl = baseUrl;

    const start = Date.now();
    try {
      const controller = new AbortController();
      const timeout = setTimeout(() => controller.abort(), 15000);

      const res = await fetch(healthUrl, {
        method: 'GET',
        headers: {
          'User-Agent': 'CPR-KeepAlive-SelfPing/3.0 (+render-anti-sleep)',
          'x-self-ping': 'true'
        },
        signal: controller.signal
      });
      clearTimeout(timeout);

      const latency = Date.now() - start;
      this.stats.totalPings++;
      this.stats.lastPingAt = new Date().toISOString();
      this.stats.lastLatencyMs = latency;

      if (res.ok) {
        this.stats.successfulPings++;
        this.stats.lastPingStatus = `OK (${res.status} in ${latency}ms)`;
        console.log(`[KeepAlive] Self-ping successful -> ${healthUrl} (${latency}ms) [Render Awake]`);
      } else {
        this.stats.failedPings++;
        this.stats.lastPingStatus = `HTTP_${res.status}`;
        console.warn(`[KeepAlive] Self-ping HTTP status ${res.status} from ${healthUrl}`);
      }
    } catch (err) {
      const latency = Date.now() - start;
      this.stats.totalPings++;
      this.stats.failedPings++;
      this.stats.lastPingAt = new Date().toISOString();
      this.stats.lastLatencyMs = latency;
      this.stats.lastPingStatus = `ERROR: ${err.message}`;
      console.warn(`[KeepAlive] Self-ping failed (${healthUrl}): ${err.message}`);
    }

    return this.stats;
  }

  start(port = 3000) {
    if (this.timer) clearInterval(this.timer);

    const baseUrl = this.getTargetUrl(port);
    this.stats.targetUrl = baseUrl;
    console.log(`[KeepAlive] Auto self-ping engine active. Target: ${baseUrl}/health | Interval: ${this.stats.intervalMinutes}m`);

    // First ping after 15 seconds to confirm connectivity, then every 10 minutes
    setTimeout(() => {
      this.ping(port);
    }, 15000);

    this.timer = setInterval(() => {
      this.ping(port);
    }, this.intervalMs);
  }

  stop() {
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }
    this.stats.enabled = false;
  }

  getStats() {
    return {
      ...this.stats,
      uptimeSeconds: Math.floor(process.uptime()),
      timestamp: Date.now()
    };
  }
}

export const keepAliveEngine = new KeepAliveEngine();
