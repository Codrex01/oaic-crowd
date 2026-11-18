#pragma once

#include <Arduino.h>

/**
 * ============================================================================
 * CROWD PRESSURE RADAR - SYSTEM CONFIGURATION & CALIBRATION
 * ============================================================================
 */

// ----------------------------------------------------------------------------
// Network & Server Connectivity
// ----------------------------------------------------------------------------
#ifndef WIFI_SSID
#define WIFI_SSID "Wokwi-GUEST"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// Server base URL without trailing slash.
// For local testing: "http://10.0.2.2:3000" or your computer's LAN IP e.g. "http://192.168.1.100:3000"
// For Wokwi cloud simulation reaching a local or deployed backend:
// Replace with your public URL (ngrok, cloudflared, Railway, or Render)
#ifndef SERVER_URL
#define SERVER_URL "http://10.0.2.2:3000"
#endif

#define DEVICE_ID        "esp32-radar-01"
#define ZONE_ID          "zone-entry-a"
#define DEVICE_KEY       "cpr-secret-key-2026"
#define FIRMWARE_VERSION "1.0.0"

// ----------------------------------------------------------------------------
// Zone Geometry & Radar Specs
// ----------------------------------------------------------------------------
// LD2450 coordinates: X from -3000 mm to +3000 mm (6 m span), Y from 0 to 6000 mm (6 m depth)
constexpr float DEFAULT_ZONE_AREA_SQM = 24.0f;  // 6m length x 4m effective corridor width = 24 m²
constexpr float CORRIDOR_WIDTH_M     = 4.0f;
constexpr uint32_t LD2450_BAUD_RATE   = 256000;

// Compile-time Mock mode fallback if hardware switch is open or undefined
#ifndef DEFAULT_MOCK_MODE
#define DEFAULT_MOCK_MODE true
#endif

// ----------------------------------------------------------------------------
// Analytics Weighting & Calibration Parameters
// ----------------------------------------------------------------------------
// Risk Score (0 - 100) = (W_DENSITY * S_d) + (W_LOW_SPEED * S_s) + 
//                        (W_STOP_GO * S_sg) + (W_CLUSTERING * S_c)
// Total weights sum strictly to 1.0
constexpr float WEIGHT_DENSITY      = 0.40f; // Critical factor: excessive crowd concentration
constexpr float WEIGHT_LOW_SPEED    = 0.30f; // Movement cessation: crowd stagnation / bottleneck
constexpr float WEIGHT_STOP_AND_GO  = 0.20f; // Turbulence proxy: jerky, stop-start shockwaves
constexpr float WEIGHT_CLUSTERING   = 0.10f; // Inter-target compression: pair distance < 0.5 m

// Dynamic Metric Normalization Baselines
constexpr float DENSITY_CRITICAL_THRESHOLD = 4.0f;  // 4.0 persons / m² equates to 100% density risk
constexpr float SPEED_NORMAL_MPS           = 1.30f; // Nominal walking speed: 1.3 m/s
constexpr float SPEED_STALL_MPS            = 0.15f; // Critical crawl/stall speed: <= 0.15 m/s (100% risk)
constexpr float STOP_GO_MAX_STDDEV         = 0.60f; // Standard deviation of speed >= 0.6 m/s is maximum shock
constexpr float CLUSTERING_DISTANCE_M      = 0.50f; // Proximity threshold for pair compression: 0.5 m (500 mm)
constexpr uint8_t SPEED_WINDOW_SIZE        = 20;    // Number of historical speed samples for std-dev

// ----------------------------------------------------------------------------
// State Machine & Hysteresis Timing (ms)
// ----------------------------------------------------------------------------
constexpr float SCORE_AMBER_THRESHOLD = 50.0f;
constexpr float SCORE_RED_THRESHOLD   = 75.0f;
constexpr float HYSTERESIS_MARGIN     = 10.0f; // Score must stay below (Threshold - 10) to de-escalate

constexpr uint32_t DURATION_ESCALATE_AMBER_MS = 5000;  // Must exceed AMBER score for 5 s
constexpr uint32_t DURATION_ESCALATE_RED_MS   = 3000;  // Must exceed RED score for 3 s
constexpr uint32_t DURATION_DEESCALATE_MS     = 10000; // Must stay below threshold - 10 for 10 s

// Gate Barrier Servo Angles & Cooldown
constexpr int SERVO_OPEN_ANGLE           = 0;     // Unobstructed flow
constexpr int SERVO_CLOSED_ANGLE         = 90;    // Barrier deployed across queue
constexpr uint32_t SERVO_RESET_DELAY_MS  = 30000; // 30 seconds of continuous GREEN before gate resets

// ----------------------------------------------------------------------------
// Periodic Task Timers (ms)
// ----------------------------------------------------------------------------
constexpr uint32_t INTERVAL_ANALYTICS_MS   = 200;   // 5 Hz analytics update
constexpr uint32_t INTERVAL_DISPLAY_PAGE_MS = 3000;  // Rotate OLED screen every 3 s
constexpr uint32_t INTERVAL_TELEMETRY_MS   = 1000;  // Post telemetry every 1 s
constexpr uint32_t INTERVAL_CMD_POLL_MS    = 2000;  // Poll backend commands every 2 s
constexpr size_t   OFFLINE_BUFFER_CAPACITY = 50;    // Queue up to 50 unsent packets when offline

