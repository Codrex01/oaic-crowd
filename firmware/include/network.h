#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "analytics.h"
#include "state_machine.h"

struct TelemetryRecord {
    uint32_t timestampMs;
    char state[12];
    float score;
    float density;
    float avgSpeed;
    float stopGoIndex;
    uint8_t targetCount;
    uint32_t entryCount;
    uint32_t exitCount;
    char alertMsg[48];
};

typedef void (*ServerCommandHandler)(const char* commandName);

class NetworkManager {
public:
    NetworkManager();

    void begin(ServerCommandHandler cmdHandler);
    void update(uint32_t now);

    bool isConnected() const { return wifiConnected; }
    void queueTelemetry(const ZoneMetrics& metrics, CrowdState state,
                        uint32_t entryCount, uint32_t exitCount,
                        const char* alertMsg);

    size_t getBufferedCount() const { return bufferCount; }

private:
    bool wifiConnected;
    uint32_t lastWifiCheckMs;
    uint32_t lastTelemetryPostMs;
    uint32_t lastCommandPollMs;
    uint32_t nextAllowedPostMs;
    uint32_t currentBackoffMs;

    ServerCommandHandler onCommandReceived;

    // Circular offline telemetry ring buffer
    TelemetryRecord offlineBuffer[OFFLINE_BUFFER_CAPACITY];
    size_t bufferHead;
    size_t bufferTail;
    size_t bufferCount;

    void checkWifiConnection(uint32_t now);
    bool postTelemetryPacket(const TelemetryRecord& record);
    void flushOfflineBuffer();
    void pollServerCommands();
};

