/**
 * ============================================================================
 * CROWD PRESSURE RADAR - COMPLETE WOKWI STANDALONE SKETCH (ENHANCED CLI & IR)
 * Privacy-First mmWave Queue Early Warning System
 * Target: ESP32 DevKit C V4
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <cmath>
#include <algorithm>

// ============================================================================
// 1. PIN DEFINITIONS
// ============================================================================
constexpr int PIN_RADAR_RX    = 16;
constexpr int PIN_RADAR_TX    = 17;
constexpr int PIN_OLED_SDA    = 21;
constexpr int PIN_OLED_SCL    = 22;
constexpr int PIN_LED_GREEN   = 25;
constexpr int PIN_LED_YELLOW  = 26;
constexpr int PIN_LED_RED     = 27;
constexpr int PIN_BUZZER      = 14;
constexpr int PIN_SERVO       = 13;
constexpr int PIN_IR_ENTRY    = 32;
constexpr int PIN_IR_EXIT     = 33;
constexpr int PIN_MOCK_TOGGLE = 4;

// ============================================================================
// 2. CONFIGURATION & CALIBRATION PARAMETERS
// ============================================================================
#define WIFI_SSID             "Wokwi-GUEST"
#define WIFI_PASSWORD         ""
#define SERVER_URL            "http://10.0.2.2:3000"
#define DEVICE_ID             "esp32-radar-01"
#define ZONE_ID               "zone-entry-a"
#define DEVICE_KEY            "cpr-secret-key-2026"
#define FIRMWARE_VERSION      "1.0.1-wokwi"

constexpr float DEFAULT_ZONE_AREA_SQM      = 24.0f;
constexpr float CORRIDOR_WIDTH_M           = 4.0f;
constexpr uint32_t LD2450_BAUD_RATE        = 256000;

// Risk Weights (Sum strictly to 1.0)
constexpr float WEIGHT_DENSITY             = 0.40f;
constexpr float WEIGHT_LOW_SPEED           = 0.30f;
constexpr float WEIGHT_STOP_AND_GO         = 0.20f;
constexpr float WEIGHT_CLUSTERING          = 0.10f;

constexpr float DENSITY_CRITICAL_THRESHOLD = 4.0f;   // 4 ppl/m²
constexpr float SPEED_NORMAL_MPS           = 1.30f;  // 1.3 m/s
constexpr float SPEED_STALL_MPS            = 0.15f;  // 0.15 m/s
constexpr float STOP_GO_MAX_STDDEV         = 0.60f;  // 0.6 m/s
constexpr float CLUSTERING_DISTANCE_MM     = 500.0f; // 0.5 m

constexpr float SCORE_AMBER_THRESHOLD      = 50.0f;
constexpr float SCORE_RED_THRESHOLD        = 75.0f;
constexpr float HYSTERESIS_MARGIN          = 10.0f;

constexpr uint32_t DURATION_ESCALATE_AMBER = 5000;
constexpr uint32_t DURATION_ESCALATE_RED   = 3000;
constexpr uint32_t DURATION_DEESCALATE     = 10000;
constexpr uint32_t SERVO_RESET_DELAY_MS    = 30000; // 30s of green before gate re-opens

constexpr int SERVO_OPEN_ANGLE             = 0;
constexpr int SERVO_CLOSED_ANGLE           = 90;

constexpr uint32_t INTERVAL_ANALYTICS_MS   = 200;
constexpr uint32_t INTERVAL_DISPLAY_MS     = 3000;
constexpr uint32_t INTERVAL_TELEMETRY_MS   = 1000;
constexpr uint32_t INTERVAL_CMD_POLL_MS    = 2000;
constexpr size_t   OFFLINE_BUFFER_CAPACITY = 50;
constexpr uint8_t  SPEED_WINDOW_SIZE       = 20;

// ============================================================================
// 3. DATA STRUCTURES
// ============================================================================
enum class CrowdState { GREEN, AMBER, RED };

struct RadarTarget {
    int16_t x;
    int16_t y;
    int16_t speed;
    uint16_t resolution;
    bool valid;
};

struct ZoneMetrics {
    float density;
    float avgSpeed;
    float stopGoIndex;
    uint16_t clusterPairs;
    float riskScore;
    uint8_t targetCount;
};

// ============================================================================
// 4. RADAR INTERFACE & DRIVERS
// ============================================================================
class IRadar {
public:
    virtual ~IRadar() = default;
    virtual void begin() = 0;
    virtual bool update() = 0;
    virtual uint8_t getTargetCount() const = 0;
    virtual const RadarTarget* getTargets() const = 0;
    virtual uint32_t getValidFrames() const = 0;
    virtual uint32_t getDroppedFrames() const = 0;
    virtual const char* getDriverName() const = 0;
};

// Real HLK-LD2450 UART Driver
class HLKLD2450Driver : public IRadar {
public:
    HLKLD2450Driver(HardwareSerial& port) : radarSerial(port), activeCount(0), validFrames(0), droppedFrames(0), bufIdx(0) {}
    void begin() override {
        radarSerial.begin(LD2450_BAUD_RATE, SERIAL_8N1, PIN_RADAR_RX, PIN_RADAR_TX);
    }
    uint8_t getTargetCount() const override { return activeCount; }
    const RadarTarget* getTargets() const override { return targets; }
    uint32_t getValidFrames() const override { return validFrames; }
    uint32_t getDroppedFrames() const override { return droppedFrames; }
    const char* getDriverName() const override { return "HLK-LD2450 UART"; }

    bool update() override {
        bool newFrame = false;
        while (radarSerial.available() > 0) {
            uint8_t b = radarSerial.read();
            if (bufIdx == 0 && b == 0xAA) rxBuf[bufIdx++] = b;
            else if (bufIdx == 1 && b == 0xFF) rxBuf[bufIdx++] = b;
            else if (bufIdx == 2 && b == 0x03) rxBuf[bufIdx++] = b;
            else if (bufIdx == 3 && b == 0x00) rxBuf[bufIdx++] = b;
            else if (bufIdx >= 4) {
                rxBuf[bufIdx++] = b;
                if (bufIdx == 30) {
                    if (rxBuf[28] == 0x55 && rxBuf[29] == 0xCC) {
                        uint8_t counted = 0;
                        for (int i = 0; i < 3; i++) {
                            int off = 4 + (i * 8);
                            uint16_t raw_x = rxBuf[off] | (rxBuf[off + 1] << 8);
                            uint16_t raw_y = rxBuf[off + 2] | (rxBuf[off + 3] << 8);
                            uint16_t raw_s = rxBuf[off + 4] | (rxBuf[off + 5] << 8);
                            uint16_t raw_r = rxBuf[off + 6] | (rxBuf[off + 7] << 8);

                            // verify against the LD2450 datasheet
                            int16_t x = (raw_x & 0x8000) ? -(int16_t)(raw_x & 0x7FFF) : (int16_t)(raw_x & 0x7FFF);
                            int16_t y = (raw_y & 0x8000) ? -(int16_t)(raw_y & 0x7FFF) : (int16_t)(raw_y & 0x7FFF);
                            int16_t s = (raw_s & 0x8000) ? -(int16_t)(raw_s & 0x7FFF) : (int16_t)(raw_s & 0x7FFF);

                            targets[i] = { x, y, s, raw_r, (raw_r > 0 && y > 50) };
                            if (targets[i].valid) counted++;
                        }
                        activeCount = counted;
                        validFrames++;
                        newFrame = true;
                    } else {
                        droppedFrames++;
                    }
                    bufIdx = 0;
                }
            } else {
                bufIdx = 0;
            }
        }
        return newFrame;
    }

private:
    HardwareSerial& radarSerial;
    RadarTarget targets[3];
    uint8_t activeCount;
    uint32_t validFrames;
    uint32_t droppedFrames;
    uint8_t rxBuf[40];
    size_t bufIdx;
};

// Synthetic Mock Radar for Wokwi Simulation
class MockRadar : public IRadar {
public:
    MockRadar() : activeCount(2), validFrames(0), lastUpdateMs(0), phaseStartMs(0), phase(0) {}
    void begin() override {
        phaseStartMs = millis();
        targets[0] = { -300, 4000, -125, 120, true };
        targets[1] = { 350, 5000, -130, 120, true };
        targets[2] = { 0, 5800, -110, 120, false };
    }
    uint8_t getTargetCount() const override { return activeCount; }
    const RadarTarget* getTargets() const override { return targets; }
    uint32_t getValidFrames() const override { return validFrames; }
    uint32_t getDroppedFrames() const override { return 0; }
    const char* getDriverName() const override { return "Synthetic Mock (Wokwi)"; }

    bool update() override {
        uint32_t now = millis();
        if (now - lastUpdateMs < 100) return false;
        lastUpdateMs = now;

        uint32_t elapsed = now - phaseStartMs;
        // Phase 0: Free Flow (15s), Phase 1: Buildup (12s), Phase 2: Stall (16s), Phase 3: Recovery (20s)
        if (phase == 0 && elapsed > 15000) { phase = 1; phaseStartMs = now; Serial.println(F("[MOCK] -> Buildup Phase (Slowing queue)")); }
        else if (phase == 1 && elapsed > 12000) { phase = 2; phaseStartMs = now; Serial.println(F("[MOCK] -> CRITICAL STALL CRUSH Phase (Bottleneck & bunching)")); }
        else if (phase == 2 && elapsed > 16000) { phase = 3; phaseStartMs = now; Serial.println(F("[MOCK] -> Recovery Phase (Crowd clearing)")); }
        else if (phase == 3 && elapsed > 20000) { phase = 0; phaseStartMs = now; Serial.println(F("[MOCK] -> Free Flow Phase (Normal flow)")); }

        if (phase == 0) { // Free flow
            activeCount = 2;
            targets[0] = { (int16_t)(-300 + sin(now/700.0)*60), (int16_t)(3500 - (now%4000)), -130, 120, true };
            targets[1] = { (int16_t)(350 + cos(now/800.0)*50), (int16_t)(5500 - (now%4500)), -125, 120, true };
            targets[2].valid = false;
        } else if (phase == 1) { // Buildup
            activeCount = 3;
            targets[0] = { -150, 2600, -65, 120, true };
            targets[1] = { 200, 3400, -55, 120, true };
            targets[2] = { -50, 4400, -60, 120, true };
        } else if (phase == 2) { // Stall / crush (targets bunch <0.5m apart, speed crawls)
            activeCount = 3;
            int16_t osc = (int16_t)(sin(now / 150.0) * 12.0);
            targets[0] = { 50, 2100, (int16_t)(-8 + osc), 150, true };
            targets[1] = { 350, 2220, (int16_t)(-5 - osc), 145, true }; // 320mm separation
            targets[2] = { -30, 2460, (int16_t)(-10 + (osc/2)), 140, true }; // 360mm separation
        } else { // Recovery
            activeCount = 2;
            targets[0] = { -300, (int16_t)(3000 - (now%3500)), -110, 120, true };
            targets[1] = { 350, (int16_t)(5000 - (now%4000)), -115, 120, true };
            targets[2].valid = false;
        }

        validFrames++;
        return true;
    }

private:
    RadarTarget targets[3];
    uint8_t activeCount;
    uint32_t validFrames;
    uint32_t lastUpdateMs;
    uint32_t phaseStartMs;
    uint8_t phase;
};

// ============================================================================
// 5. ANALYTICS & STATE MACHINE
// ============================================================================
class AnalyticsEngine {
public:
    AnalyticsEngine() : speedIdx(0), speedCount(0) {
        for (int i = 0; i < SPEED_WINDOW_SIZE; i++) speedWin[i] = 0.0f;
    }
    void reset() {
        speedIdx = 0; speedCount = 0;
        lastMetrics = { 0, 0, 0, 0, 0, 0 };
    }
    ZoneMetrics process(const RadarTarget* targets, uint8_t count) {
        uint8_t validCount = 0;
        float speedSum = 0.0f;
        for (int i = 0; i < count; i++) {
            if (targets[i].valid) {
                validCount++;
                speedSum += (std::abs((float)targets[i].speed) / 100.0f);
            }
        }
        float avgSpeed = (validCount > 0) ? (speedSum / validCount) : 0.0f;
        float density = (float)validCount / DEFAULT_ZONE_AREA_SQM;

        // Proximity clustering (< 500 mm)
        uint16_t clusters = 0;
        for (int i = 0; i < count; i++) {
            if (!targets[i].valid) continue;
            for (int j = i + 1; j < count; j++) {
                if (!targets[j].valid) continue;
                float dx = targets[i].x - targets[j].x;
                float dy = targets[i].y - targets[j].y;
                if (std::sqrt(dx*dx + dy*dy) < CLUSTERING_DISTANCE_MM) clusters++;
            }
        }

        // Sliding stop-and-go std-dev
        speedWin[speedIdx] = avgSpeed;
        speedIdx = (speedIdx + 1) % SPEED_WINDOW_SIZE;
        if (speedCount < SPEED_WINDOW_SIZE) speedCount++;

        float stdDev = 0.0f;
        if (speedCount >= 2) {
            float mean = 0.0f;
            for (int i = 0; i < speedCount; i++) mean += speedWin[i];
            mean /= speedCount;
            float var = 0.0f;
            for (int i = 0; i < speedCount; i++) {
                float diff = speedWin[i] - mean;
                var += (diff * diff);
            }
            stdDev = std::sqrt(var / speedCount);
        }

        // Composite risk formulation
        float s_dens = std::min(100.0f, (density / DENSITY_CRITICAL_THRESHOLD) * 100.0f);
        float s_speed = 0.0f;
        if (avgSpeed <= SPEED_STALL_MPS) s_speed = 100.0f;
        else if (avgSpeed < SPEED_NORMAL_MPS) s_speed = ((SPEED_NORMAL_MPS - avgSpeed) / (SPEED_NORMAL_MPS - SPEED_STALL_MPS)) * 100.0f;

        float s_stopgo = std::min(100.0f, (stdDev / STOP_GO_MAX_STDDEV) * 100.0f);
        float s_clust = std::min(100.0f, (float)clusters * 50.0f);

        float score = (WEIGHT_DENSITY * s_dens) + (WEIGHT_LOW_SPEED * s_speed) +
                      (WEIGHT_STOP_AND_GO * s_stopgo) + (WEIGHT_CLUSTERING * s_clust);
        score = std::min(100.0f, std::max(0.0f, score));

        lastMetrics = { density, avgSpeed, stdDev, clusters, score, validCount };
        return lastMetrics;
    }
    const ZoneMetrics& getMetrics() const { return lastMetrics; }

private:
    float speedWin[SPEED_WINDOW_SIZE];
    uint8_t speedIdx;
    uint8_t speedCount;
    ZoneMetrics lastMetrics;
};

// State Machine with Hysteresis
class StateMachine {
public:
    StateMachine() : state(CrowdState::GREEN), enteredMs(0), greenStartMs(0), redCand(0), ambCand(0), deescCand(0) {}
    void begin() {
        state = CrowdState::GREEN;
        enteredMs = millis();
        greenStartMs = enteredMs;
        redCand = 0; ambCand = 0; deescCand = 0;
    }
    CrowdState getState() const { return state; }
    const char* getStateName() const {
        return (state == CrowdState::RED) ? "RED" : (state == CrowdState::AMBER ? "AMBER" : "GREEN");
    }
    uint32_t getGreenDuration(uint32_t now) const {
        return (state == CrowdState::GREEN) ? (now - greenStartMs) : 0;
    }
    bool update(float score, uint32_t now) {
        CrowdState prev = state;
        if (state == CrowdState::GREEN) {
            if (score >= SCORE_RED_THRESHOLD) {
                if (redCand == 0) redCand = now;
                else if (now - redCand >= DURATION_ESCALATE_RED) transition(CrowdState::RED, score, now, "Score >= 75 for 3s");
            } else redCand = 0;

            if (score >= SCORE_AMBER_THRESHOLD) {
                if (ambCand == 0) ambCand = now;
                else if (now - ambCand >= DURATION_ESCALATE_AMBER) transition(CrowdState::AMBER, score, now, "Score >= 50 for 5s");
            } else ambCand = 0;
        } else if (state == CrowdState::AMBER) {
            if (score >= SCORE_RED_THRESHOLD) {
                if (redCand == 0) redCand = now;
                else if (now - redCand >= DURATION_ESCALATE_RED) transition(CrowdState::RED, score, now, "Score >= 75 for 3s");
            } else redCand = 0;

            if (score < (SCORE_AMBER_THRESHOLD - HYSTERESIS_MARGIN)) {
                if (deescCand == 0) deescCand = now;
                else if (now - deescCand >= DURATION_DEESCALATE) transition(CrowdState::GREEN, score, now, "Score < 40 for 10s");
            } else deescCand = 0;
        } else if (state == CrowdState::RED) {
            if (score < (SCORE_RED_THRESHOLD - HYSTERESIS_MARGIN)) {
                if (deescCand == 0) deescCand = now;
                else if (now - deescCand >= DURATION_DEESCALATE) transition(CrowdState::AMBER, score, now, "Score < 65 for 10s");
            } else deescCand = 0;
        }
        return (state != prev);
    }
    void force(CrowdState s, const char* r) {
        transition(s, 0, millis(), r);
    }

private:
    CrowdState state;
    uint32_t enteredMs;
    uint32_t greenStartMs;
    uint32_t redCand, ambCand, deescCand;
    void transition(CrowdState s, float score, uint32_t now, const char* reason) {
        CrowdState p = state;
        state = s;
        enteredMs = now;
        if (s == CrowdState::GREEN) greenStartMs = now;
        else greenStartMs = 0;
        redCand = 0; ambCand = 0; deescCand = 0;
        Serial.printf("[STATE TRANSITION] [%u ms] %s -> %s (Score: %.1f) | %s\n",
                      now, (p==CrowdState::RED?"RED":(p==CrowdState::AMBER?"AMBER":"GREEN")),
                      (s==CrowdState::RED?"RED":(s==CrowdState::AMBER?"AMBER":"GREEN")), score, reason);
    }
};

// ============================================================================
// 6. ACTUATORS, OLED DISPLAY & SYSTEM INSTANCES
// ============================================================================
static HLKLD2450Driver realRadar(Serial2);
static MockRadar       mockRadar;
static IRadar*         radar = &mockRadar;

static AnalyticsEngine analytics;
static StateMachine    fsm;
static Servo           gateServo;
static Adafruit_SSD1306 oled(128, 64, &Wire, -1);

static volatile uint32_t entryCount = 0;
static volatile uint32_t exitCount  = 0;
static uint32_t lastEntryTrigger    = 0;
static uint32_t lastExitTrigger     = 0;
static volatile bool entryBeamTriggered = false;
static volatile bool exitBeamTriggered  = false;

static int  currentGateAngle    = SERVO_OPEN_ANGLE;
static bool buzzerMuted         = false;
static uint32_t amberBeepUntil  = 0;
static uint32_t lastBlinkToggle = 0;
static bool blinkOn             = false;

static uint32_t lastAnalyticsMs = 0;
static uint32_t lastDisplayMs   = 0;
static uint32_t lastTelemetryMs = 0;
static uint8_t  oledPage        = 0;
static bool     wifiOk          = false;

void IRAM_ATTR isrEntry() {
    uint32_t now = millis();
    if (now - lastEntryTrigger > 250) {
        entryCount++;
        lastEntryTrigger = now;
        entryBeamTriggered = true;
    }
}
void IRAM_ATTR isrExit() {
    uint32_t now = millis();
    if (now - lastExitTrigger > 250) {
        exitCount++;
        lastExitTrigger = now;
        exitBeamTriggered = true;
    }
}

void setGate(int angle, const char* reason) {
    gateServo.write(angle);
    currentGateAngle = angle;
    Serial.printf("[SERVO GATE] Angle -> %d deg (%s) | Reason: %s\n",
                  angle, (angle >= 45 ? "CLOSED [LOCKOUT]" : "OPEN [CLEAR]"), reason);
}

void renderOled(uint32_t now) {
    if (now - lastDisplayMs >= INTERVAL_DISPLAY_MS) {
        lastDisplayMs = now;
        oledPage = (oledPage + 1) % 3;
    }
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);

    ZoneMetrics m = analytics.getMetrics();

    if (oledPage == 0) {
        oled.setCursor(0, 0); oled.print(ZONE_ID);
        oled.setCursor(80, 0); oled.print(wifiOk ? "WiFi:OK" : "WiFi:--");
        oled.drawLine(0, 9, 127, 9, SSD1306_WHITE);

        oled.setCursor(0, 14); oled.print("STATE: ");
        oled.setTextSize(2); oled.setCursor(45, 12);
        oled.print(fsm.getStateName());

        oled.setTextSize(1); oled.setCursor(0, 34);
        oled.printf("Risk Score: %.0f/100", m.riskScore);
        oled.drawRect(0, 46, 128, 10, SSD1306_WHITE);
        int fill = (int)((m.riskScore / 100.0f) * 124.0f);
        if (fill > 0) oled.fillRect(2, 48, fill, 6, SSD1306_WHITE);

        oled.setCursor(0, 57); oled.printf("Targets: %u active", m.targetCount);
    } else if (oledPage == 1) {
        oled.setCursor(0, 0); oled.print("-- CROWD METRICS --");
        oled.drawLine(0, 9, 127, 9, SSD1306_WHITE);
        oled.setCursor(0, 14); oled.printf("Density: %.2f p/m2", m.density);
        oled.setCursor(0, 26); oled.printf("Speed:   %.2f m/s", m.avgSpeed);
        oled.setCursor(0, 38); oled.printf("StopGo:  %.2f m/s", m.stopGoIndex);
        oled.setCursor(0, 50); oled.printf("Clusters: %u (<0.5m)", m.clusterPairs);
    } else {
        oled.setCursor(0, 0); oled.print("-- CORRIDOR FLOW --");
        oled.drawLine(0, 9, 127, 9, SSD1306_WHITE);
        oled.setCursor(0, 14); oled.printf("IR In:  %u", entryCount);
        oled.setCursor(68, 14); oled.printf("Out: %u", exitCount);
        int32_t net = (int32_t)entryCount - (int32_t)exitCount;
        oled.setCursor(0, 28); oled.printf("Net In: %ld ppl", net < 0 ? 0 : net);
        oled.setCursor(0, 42); oled.printf("Gate:   %s", currentGateAngle >= 45 ? "CLOSED [90]" : "OPEN [0]");
        oled.setCursor(0, 54); oled.printf("Radar:  %s", radar->getDriverName());
    }
    oled.display();
}

// ============================================================================
// 7. SETUP & MAIN EVENT LOOP
// ============================================================================
void setup() {
    Serial.begin(115200);
    delay(200);

    Serial.println(F("\n========================================================"));
    Serial.println(F("   CROWD PRESSURE RADAR - EMBEDDED NODE INITIALIZING   "));
    Serial.println(F("   CLI active. Type 'help' for command list.            "));
    Serial.println(F("========================================================"));

    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
    pinMode(PIN_MOCK_TOGGLE, INPUT_PULLUP);
    pinMode(PIN_IR_ENTRY, INPUT_PULLUP);
    pinMode(PIN_IR_EXIT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_IR_ENTRY), isrEntry, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_IR_EXIT), isrExit, FALLING);

    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        oled.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    }
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(15, 25);
    oled.println(F("CROWD RADAR BOOT"));
    oled.display();

    ESP32PWM::allocateTimer(0);
    gateServo.setPeriodHertz(50);
    gateServo.attach(PIN_SERVO, 500, 2400);
    setGate(SERVO_OPEN_ANGLE, "Boot state");

    realRadar.begin();
    mockRadar.begin();
    radar = (digitalRead(PIN_MOCK_TOGGLE) == LOW) ? (IRadar*)&mockRadar : (IRadar*)&mockRadar;

    fsm.begin();
    analytics.reset();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
    uint32_t now = millis();

    // 1. Process IR Beam Button Triggers (instant feedback to Serial)
    if (entryBeamTriggered) {
        entryBeamTriggered = false;
        int32_t net = (int32_t)entryCount - (int32_t)exitCount;
        Serial.printf("[IR BEAM A] Entry detected! Total In: %u | Net in corridor: %ld ppl\n", entryCount, net < 0 ? 0 : net);
    }
    if (exitBeamTriggered) {
        exitBeamTriggered = false;
        int32_t net = (int32_t)entryCount - (int32_t)exitCount;
        Serial.printf("[IR BEAM B] Exit detected! Total Out: %u | Net in corridor: %ld ppl\n", exitCount, net < 0 ? 0 : net);
    }

    // 2. Hardware Mock Toggle Switch (GPIO4)
    static bool lastMockPinState = HIGH;
    bool currentMockPin = digitalRead(PIN_MOCK_TOGGLE);
    if (currentMockPin != lastMockPinState) {
        lastMockPinState = currentMockPin;
        if (currentMockPin == LOW) {
            radar = &mockRadar;
            Serial.println(F("[SWITCH GPIO4] Pressed -> Switched to SYNTHETIC MOCK model"));
        } else {
            radar = &realRadar;
            Serial.println(F("[SWITCH GPIO4] Released -> Switched to PHYSICAL UART RADAR"));
        }
    }

    // 3. Serial CLI Command Processing with Full Feedback
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd == "status") {
            ZoneMetrics m = analytics.getMetrics();
            Serial.println(F("\n+---------------- SYSTEM STATUS REPORT ----------------+"));
            Serial.printf("| Current State: %-10s (Risk Score: %5.1f / 100) |\n", fsm.getStateName(), m.riskScore);
            Serial.printf("| Crowd Density: %4.2f p/m²   | Avg Speed: %4.2f m/s         |\n", m.density, m.avgSpeed);
            Serial.printf("| Stop&Go StdDev: %4.2f m/s  | Clustering: %u pairs (<0.5m) |\n", m.stopGoIndex, m.clusterPairs);
            Serial.printf("| Gate Position: %-6s    | Servo Angle: %2d deg         |\n", currentGateAngle >= 45 ? "CLOSED" : "OPEN", currentGateAngle);
            Serial.printf("| IR Flow Count: In=%-5u  | Out=%-5u (Net=%-4d)        |\n", entryCount, exitCount, (int)(entryCount - exitCount));
            Serial.printf("| Radar Source:  %-36s |\n", radar->getDriverName());
            Serial.println(F("+------------------------------------------------------+\n"));
        } else if (cmd == "mock on") {
            radar = &mockRadar;
            Serial.println(F("[CLI] Switched radar to SYNTHETIC MOCK model."));
        } else if (cmd == "mock off") {
            radar = &realRadar;
            Serial.println(F("[CLI] Switched radar to PHYSICAL HLK-LD2450 UART."));
        } else if (cmd == "force red") {
            fsm.force(CrowdState::RED, "CLI override: force red");
            setGate(SERVO_CLOSED_ANGLE, "CLI override: force red");
            Serial.println(F("[CLI] Forced state to RED! Servo locked to 90 deg, buzzer active, Red LED blinking."));
        } else if (cmd == "force green") {
            fsm.force(CrowdState::GREEN, "CLI override: force green");
            setGate(SERVO_OPEN_ANGLE, "CLI override: force green");
            Serial.println(F("[CLI] Forced state to GREEN! Servo opened to 0 deg, alarm silenced."));
        } else if (cmd == "gate open") {
            setGate(SERVO_OPEN_ANGLE, "Manual CLI command: gate open");
            Serial.println(F("[CLI] Gate barrier manually opened to 0 deg."));
        } else if (cmd == "gate close") {
            setGate(SERVO_CLOSED_ANGLE, "Manual CLI command: gate close");
            Serial.println(F("[CLI] Gate barrier manually closed to 90 deg."));
        } else if (cmd == "help") {
            Serial.println(F("\n[CLI HELP] Available commands:"));
            Serial.println(F("  status      - Display full real-time telemetry diagnostics"));
            Serial.println(F("  force red   - Force critical RED state & close servo gate"));
            Serial.println(F("  force green - Force clear GREEN state & open servo gate"));
            Serial.println(F("  gate open   - Manually open servo gate barrier (0 deg)"));
            Serial.println(F("  gate close  - Manually close servo gate barrier (90 deg)"));
            Serial.println(F("  mock on     - Enable synthetic queue stall simulation"));
            Serial.println(F("  mock off    - Switch to physical HLK-LD2450 radar\n"));
        } else if (cmd.length() > 0) {
            Serial.printf("[CLI] Unknown command: '%s'. Type 'help' for commands.\n", cmd.c_str());
        }
    }

    // 4. Update Radar & Analytics
    radar->update();
    if (now - lastAnalyticsMs >= INTERVAL_ANALYTICS_MS) {
        lastAnalyticsMs = now;
        ZoneMetrics m = analytics.process(radar->getTargets(), radar->getTargetCount());
        bool changed = fsm.update(m.riskScore, now);
        if (changed) {
            if (fsm.getState() == CrowdState::RED) {
                setGate(SERVO_CLOSED_ANGLE, "Automatic RED State Lockout - Emergency Crowd Diversion");
            } else if (fsm.getState() == CrowdState::AMBER) {
                amberBeepUntil = now + 200;
            }
        }
    }

    // 5. Automatic Gate Barrier Release (30s continuous GREEN)
    if (currentGateAngle == SERVO_CLOSED_ANGLE) {
        if (fsm.getState() == CrowdState::GREEN && fsm.getGreenDuration(now) >= SERVO_RESET_DELAY_MS) {
            setGate(SERVO_OPEN_ANGLE, "Continuous GREEN > 30s: Safe to resume queue flow");
        }
    }

    // 6. Update LEDs & Buzzer
    CrowdState st = fsm.getState();
    if (st == CrowdState::GREEN) {
        digitalWrite(PIN_LED_GREEN, HIGH);
        digitalWrite(PIN_LED_YELLOW, LOW);
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_BUZZER, LOW);
    } else if (st == CrowdState::AMBER) {
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_YELLOW, HIGH);
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_BUZZER, (!buzzerMuted && now < amberBeepUntil) ? HIGH : LOW);
    } else { // RED
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_YELLOW, LOW);
        if (now - lastBlinkToggle >= 250) {
            lastBlinkToggle = now;
            blinkOn = !blinkOn;
            digitalWrite(PIN_LED_RED, blinkOn ? HIGH : LOW);
        }
        digitalWrite(PIN_BUZZER, (!buzzerMuted && ((now / 250) % 2 == 0)) ? HIGH : LOW);
    }

    // 7. Periodic OLED Display Rendering
    renderOled(now);
}
