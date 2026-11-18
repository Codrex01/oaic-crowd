#include "network.h"

NetworkManager::NetworkManager()
    : wifiConnected(false),
      lastWifiCheckMs(0),
      lastTelemetryPostMs(0),
      lastCommandPollMs(0),
      nextAllowedPostMs(0),
      currentBackoffMs(1000),
      onCommandReceived(nullptr),
      bufferHead(0),
      bufferTail(0),
      bufferCount(0) {}

void NetworkManager::begin(ServerCommandHandler cmdHandler) {
    onCommandReceived = cmdHandler;
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.printf("[NETWORK] Connecting to WiFi SSID: '%s'...\n", WIFI_SSID);
    lastWifiCheckMs = millis();
}

void NetworkManager::checkWifiConnection(uint32_t now) {
    if (now - lastWifiCheckMs >= 3000) {
        lastWifiCheckMs = now;
        bool currentlyConnected = (WiFi.status() == WL_CONNECTED);

        if (currentlyConnected && !wifiConnected) {
            wifiConnected = true;
            Serial.printf("[NETWORK] WiFi Connected! IP: %s | Server Target: %s\n",
                          WiFi.localIP().toString().c_str(), SERVER_URL);
            flushOfflineBuffer();
        } else if (!currentlyConnected && wifiConnected) {
            wifiConnected = false;
            Serial.println(F("[NETWORK] WiFi Connection Lost. Switching to offline queue."));
        }
    }
}

void NetworkManager::queueTelemetry(const ZoneMetrics& metrics, CrowdState state,
                                    uint32_t entryCount, uint32_t exitCount,
                                    const char* alertMsg) {
    TelemetryRecord record;
    record.timestampMs = millis();
    strncpy(record.state, CrowdStateMachine::getStateName(state), sizeof(record.state) - 1);
    record.state[sizeof(record.state) - 1] = '\0';
    record.score = metrics.riskScore;
    record.density = metrics.density;
    record.avgSpeed = metrics.avgSpeed;
    record.stopGoIndex = metrics.stopGoIndex;
    record.targetCount = metrics.targetCount;
    record.entryCount = entryCount;
    record.exitCount = exitCount;

    if (alertMsg != nullptr && strlen(alertMsg) > 0) {
        strncpy(record.alertMsg, alertMsg, sizeof(record.alertMsg) - 1);
        record.alertMsg[sizeof(record.alertMsg) - 1] = '\0';
    } else {
        record.alertMsg[0] = '\0';
    }

    if (bufferCount < OFFLINE_BUFFER_CAPACITY) {
        offlineBuffer[bufferHead] = record;
        bufferHead = (bufferHead + 1) % OFFLINE_BUFFER_CAPACITY;
        bufferCount++;
    } else {
        // Buffer full: drop oldest sample to prioritize recent telemetry
        offlineBuffer[bufferHead] = record;
        bufferHead = (bufferHead + 1) % OFFLINE_BUFFER_CAPACITY;
        bufferTail = (bufferTail + 1) % OFFLINE_BUFFER_CAPACITY;
    }
}

bool NetworkManager::postTelemetryPacket(const TelemetryRecord& record) {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    char endpoint[128];
    snprintf(endpoint, sizeof(endpoint), "%s/api/telemetry", SERVER_URL);

    HTTPClient http;
    http.begin(endpoint);
    http.setTimeout(1500); // 1.5s timeout for low latency
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-Key", DEVICE_KEY);

    JsonDocument doc;
    doc["deviceId"] = DEVICE_ID;
    doc["zoneId"] = ZONE_ID;
    doc["timestamp"] = record.timestampMs;
    doc["state"] = record.state;
    doc["score"] = serialized(String(record.score, 1));
    doc["density"] = serialized(String(record.density, 2));
    doc["avgSpeed"] = serialized(String(record.avgSpeed, 2));
    doc["stopGoIndex"] = serialized(String(record.stopGoIndex, 2));
    doc["targetCount"] = record.targetCount;
    doc["entryCount"] = record.entryCount;
    doc["exitCount"] = record.exitCount;
    doc["firmwareVersion"] = FIRMWARE_VERSION;

    JsonArray alertsArr = doc["activeAlerts"].to<JsonArray>();
    if (strlen(record.alertMsg) > 0) {
        alertsArr.add(record.alertMsg);
    } else if (strcmp(record.state, "RED") == 0) {
        alertsArr.add("CRITICAL_PRESSURE_WARNING");
    } else if (strcmp(record.state, "AMBER") == 0) {
        alertsArr.add("ELEVATED_CONGESTION");
    }

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    int httpCode = http.POST(jsonPayload);
    bool success = (httpCode >= 200 && httpCode < 300);

    if (success) {
        currentBackoffMs = 1000; // Reset backoff
    } else {
        // Exponential backoff up to 16 seconds
        currentBackoffMs = std::min(16000UL, currentBackoffMs * 2);
        nextAllowedPostMs = millis() + currentBackoffMs;
        Serial.printf("[HTTP POST] Failed (code=%d). Backing off %u ms\n", httpCode, currentBackoffMs);
    }

    http.end();
    return success;
}

void NetworkManager::flushOfflineBuffer() {
    if (bufferCount == 0 || WiFi.status() != WL_CONNECTED) {
        return;
    }

    Serial.printf("[NETWORK] Flushing %u queued packets to server...\n", (unsigned int)bufferCount);
    while (bufferCount > 0 && WiFi.status() == WL_CONNECTED) {
        const TelemetryRecord& record = offlineBuffer[bufferTail];
        if (postTelemetryPacket(record)) {
            bufferTail = (bufferTail + 1) % OFFLINE_BUFFER_CAPACITY;
            bufferCount--;
        } else {
            break; // Stop flushing if transmission failed
        }
    }
}

void NetworkManager::pollServerCommands() {
    if (WiFi.status() != WL_CONNECTED || onCommandReceived == nullptr) {
        return;
    }

    char endpoint[128];
    snprintf(endpoint, sizeof(endpoint), "%s/api/commands/%s", SERVER_URL, DEVICE_ID);

    HTTPClient http;
    http.begin(endpoint);
    http.setTimeout(1200);
    http.addHeader("X-Device-Key", DEVICE_KEY);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        if (payload.length() > 5) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, payload);
            if (!err) {
                const char* cmd = doc["command"];
                if (cmd != nullptr && strlen(cmd) > 0) {
                    Serial.printf("[SERVER COMMAND RECEIVED] -> %s\n", cmd);
                    onCommandReceived(cmd);
                }
            }
        }
    }

    http.end();
}

void NetworkManager::update(uint32_t now) {
    checkWifiConnection(now);

    // Periodically post queued telemetry
    if (now >= nextAllowedPostMs && (now - lastTelemetryPostMs >= INTERVAL_TELEMETRY_MS)) {
        lastTelemetryPostMs = now;

        if (bufferCount > 0) {
            const TelemetryRecord& rec = offlineBuffer[bufferTail];
            if (postTelemetryPacket(rec)) {
                bufferTail = (bufferTail + 1) % OFFLINE_BUFFER_CAPACITY;
                bufferCount--;
            }
        }
    }

    // Periodically poll for operator commands (every 2s)
    if (now - lastCommandPollMs >= INTERVAL_CMD_POLL_MS) {
        lastCommandPollMs = now;
        pollServerCommands();
    }
}

