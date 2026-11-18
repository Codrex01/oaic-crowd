#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pins.h"
#include "config.h"
#include "analytics.h"
#include "state_machine.h"
#include "actuators.h"

class DisplayManager {
public:
    DisplayManager();

    bool begin();
    void update(CrowdState state, const ZoneMetrics& metrics,
                uint32_t entryCount, uint32_t exitCount,
                GatePosition gatePos, bool wifiConnected,
                const char* radarDriverName, uint32_t now);

private:
    Adafruit_SSD1306 display;
    bool displayAvailable;
    uint8_t currentPage;
    uint32_t lastPageRotateMs;

    void renderPage0(CrowdState state, const ZoneMetrics& metrics, bool wifiConnected);
    void renderPage1(const ZoneMetrics& metrics);
    void renderPage2(uint32_t entryCount, uint32_t exitCount, GatePosition gatePos, const char* radarDriverName);
};

