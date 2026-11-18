#include "display.h"

DisplayManager::DisplayManager()
    : display(128, 64, &Wire, -1),
      displayAvailable(false),
      currentPage(0),
      lastPageRotateMs(0) {}

bool DisplayManager::begin() {
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

    // Try primary SSD1306 address 0x3C, fallback to 0x3D
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
            Serial.println(F("[OLED] Warning: SSD1306 display not detected on I2C bus."));
            displayAvailable = false;
            return false;
        }
    }

    displayAvailable = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(10, 15);
    display.println(F("CROWD RADAR"));
    display.setCursor(10, 30);
    display.println(F("Early Warning Sys"));
    display.setCursor(10, 45);
    display.println(F("Booting sensor..."));
    display.display();
    delay(500);

    lastPageRotateMs = millis();
    Serial.println(F("[OLED] SSD1306 display initialized successfully."));
    return true;
}

void DisplayManager::renderPage0(CrowdState state, const ZoneMetrics& metrics, bool wifiConnected) {
    display.clearDisplay();

    // Top status line: Zone ID and WiFi indicator
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(ZONE_ID);

    display.setCursor(85, 0);
    display.print(wifiConnected ? F("WiFi:OK") : F("WiFi:--"));
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    // Large State Banner
    display.setCursor(0, 14);
    display.print(F("STATE: "));
    display.setTextSize(2);
    display.setCursor(45, 12);
    switch (state) {
        case CrowdState::GREEN: display.print(F("GREEN")); break;
        case CrowdState::AMBER: display.print(F("AMBER")); break;
        case CrowdState::RED:   display.print(F("RED!!")); break;
    }

    // Risk score progress bar and numbers
    display.setTextSize(1);
    display.setCursor(0, 34);
    display.printf("Risk Score: %.0f/100", metrics.riskScore);

    // Draw horizontal score bar (outline + fill)
    display.drawRect(0, 46, 128, 10, SSD1306_WHITE);
    int fillWidth = static_cast<int>((metrics.riskScore / 100.0f) * 124.0f);
    if (fillWidth > 124) fillWidth = 124;
    if (fillWidth > 0) {
        display.fillRect(2, 48, fillWidth, 6, SSD1306_WHITE);
    }

    display.setCursor(0, 57);
    display.printf("Targets: %u in zone", metrics.targetCount);

    display.display();
}

void DisplayManager::renderPage1(const ZoneMetrics& metrics) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("-- CROWD DYNAMICS --"));
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    display.setCursor(0, 14);
    display.printf("Density:   %.2f p/m2", metrics.density);

    display.setCursor(0, 26);
    display.printf("Avg Speed: %.2f m/s", metrics.avgSpeed);

    display.setCursor(0, 38);
    display.printf("Stop&Go:   %.2f m/s", metrics.stopGoIndex);

    display.setCursor(0, 50);
    display.printf("Clusters:  %u (<0.5m)", metrics.clusterPairs);

    display.display();
}

void DisplayManager::renderPage2(uint32_t entryCount, uint32_t exitCount, GatePosition gatePos, const char* radarDriverName) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("-- CORRIDOR FLOW --"));
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    int32_t inZoneNet = static_cast<int32_t>(entryCount) - static_cast<int32_t>(exitCount);
    if (inZoneNet < 0) inZoneNet = 0;

    display.setCursor(0, 14);
    display.printf("IR In:  %u", entryCount);

    display.setCursor(68, 14);
    display.printf("Out: %u", exitCount);

    display.setCursor(0, 28);
    display.printf("Net In-Zone: %ld ppl", inZoneNet);

    display.setCursor(0, 42);
    display.printf("Gate: %s", (gatePos == GatePosition::CLOSED ? "CLOSED [90]" : "OPEN [0]"));

    display.setCursor(0, 54);
    display.printf("Radar: %s", (strstr(radarDriverName, "Synthetic") ? "MOCK" : "LD2450"));

    display.display();
}

void DisplayManager::update(CrowdState state, const ZoneMetrics& metrics,
                            uint32_t entryCount, uint32_t exitCount,
                            GatePosition gatePos, bool wifiConnected,
                            const char* radarDriverName, uint32_t now) {
    if (!displayAvailable) return;

    // Rotate OLED screens every INTERVAL_DISPLAY_PAGE_MS (3000 ms)
    if (now - lastPageRotateMs >= INTERVAL_DISPLAY_PAGE_MS) {
        lastPageRotateMs = now;
        currentPage = (currentPage + 1) % 3;
    }

    switch (currentPage) {
        case 0: renderPage0(state, metrics, wifiConnected); break;
        case 1: renderPage1(metrics); break;
        case 2: renderPage2(entryCount, exitCount, gatePos, radarDriverName); break;
    }
}

