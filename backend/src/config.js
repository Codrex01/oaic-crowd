import dotenv from 'dotenv';
dotenv.config();

export const config = {
  port: parseInt(process.env.PORT || '3000', 10),
  deviceKey: process.env.DEVICE_KEY || 'cpr-secret-key-2026',
  corsOrigin: process.env.CORS_ORIGIN || '*',
  dataDir: process.env.DATA_DIR || './data',
  rateLimitReqPerSec: parseInt(process.env.RATE_LIMIT_REQ_PER_SEC || '2', 10),
  historyRetentionMinutes: 10,
  maxPointsPerZone: 1000,
  commandExpiryMs: 30000 // 30 seconds
};

