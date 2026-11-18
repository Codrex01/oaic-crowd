#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "radar_driver.h"
#include "mock_radar.h"
#include "analytics.h"
#include "state_machine.h"
#include "actuators.h"
#include "display.h"
#include "network.h"

// ============================================================================
// SYSTEM INSTANCES & GLOBAL STATE
// ============================================================================
static HLKLD2450Driver realRadar(Serial2, PIN_RADAR_RX, PIN_RADAR_TX, LD2450_BAUD_RATE);
static MockRadar       mockRadar;
static IRadar*         activeRadar = nullptr;

static AnalyticsEngine      analytics(DEFAULT_ZONE_AREA_SQM);
static CrowdStateMachine    stateMachine;
static ActuatorController   actuators;
static DisplayManager       displayMgr;
static NetworkManager       networkMgr;

// IR Break-Beam Counters
static volatile uint32_t irEntryCount = 0;
static volatile uint32_t irExitCount  = 0;
static uint32_t lastEntryTriggerMs    = 0;
static uint32_t lastExitTriggerMs     = 0;

// Periodic Execution Timers
static uint32_t lastAnalyticsRunMs   = 0;
static uint32_t lastTelemetryQueueMs = 0;
static bool     isMockModeActive     = DEFAULT_MOCK_MODE;
static char     serialCmdBuffer[64];
static size_t   serialCmdIndex       = 0;

// ============================================================================
// IR BREAK-BEAM INTERRUPT SERVICE ROUTINES (WITH 250ms SOFTWARE DEBOUNCE)
// ============================================================================
void IRAM_ATTR isrEntryBeam() {
    uint32_t now = millis();
    if (now - lastEntryTriggerMs > 250) {
        irEntryCount++;
        lastEntryTriggerMs = now;
    }
}

void IRAM_ATTR isrExitBeam() {
    uint32_t now = millis();
    if (now - lastExitTriggerMs > 250) {
        irExitCount++;
        lastExitTriggerMs = now;
    }
}

// ============================================================================
// SERVER COMMAND DISPATCHER
// ============================================================================
void handleServerCommand(const char* command) {
    if (command == nullptr) return;

    if (strcmp(command, "MANUAL_GATE_OPEN") == 0) {
        actuators.manualSetGate(GatePosition::OPEN, "Server Command: MANUAL_GATE_OPEN");
    } else if (strcmp(command, "MANUAL_GATE_CLOSE") == 0) {
        actuators.manualSetGate(GatePosition::CLOSED, "Server Command: MANUAL_GATE_CLOSE");
    } else if (strcmp(command, "SILENCE_BUZZER") == 0) {
        actuators.silenceBuzzer(true);
    } else if (strcmp(command, "RESET_ZONE") == 0) {
        Serial.println(F("[COMMAND] Executing RESET_ZONE: clearing metrics & resetting gate"));
        stateMachine.reset();
        analytics.reset();
        irEntryCount = 0;
        irExitCount = 0;
        actuators.manualSetGate(GatePosition::OPEN, "Server Command: RESET_ZONE");
        actuators.silenceBuzzer(false);
    } else {
        Serial.printf("[COMMAND] Unrecognized server command: '%s'\n", command);
    }
}

// ============================================================================
// SERIAL CLI COMMAND HANDLER
// ============================================================================
void processSerialCommand(const char* cmd) {
    if (strcmp(cmd, "status") == 0) {
        uint32_t now = millis();
        ZoneMetrics m = analytics.getLastMetrics();
        Serial.println(F("\n================= SYSTEM STATUS ================="));
        Serial.printf("Device ID:      %s (Zone: %s)\n", DEVICE_ID, ZONE_ID);
        Serial.printf("Firmware:       v%s\n", FIRMWARE_VERSION);
        Serial.printf("Radar Source:   %s\n", activeRadar->getDriverName());
        Serial.printf("Radar Stats:    Valid: %u, Dropped: %u\n", activeRadar->getValidFrames(), activeRadar->getDroppedFrames());
        Serial.printf("Current State:  %s (Duration: %u s)\n", stateMachine.getCurrentStateName(), stateMachine.getTimeInCurrentState(now) / 1000);
        Serial.printf("Risk Score:     %.1f / 100.0\n", m.riskScore);
        Serial.printf("Density:        %.2f ppl/m² (%u targets in zone)\n", m.density, m.targetCount);
        Serial.printf("Avg Speed:      %.2f m/s (Stop-Go Index: %.2f m/s)\n", m.avgSpeed, m.stopGoIndex);
        Serial.printf("Clustering:     %u compressed pairs (<0.5m)\n", m.clusterPairs);
        Serial.printf("IR Flow:        Entry=%u, Exit=%u (Net: %d)\n", irEntryCount, irExitCount, (int)(irEntryCount - irExitCount));
        Serial.printf("Gate Barrier:   %s\n", (actuators.getGatePosition() == GatePosition::CLOSED ? "CLOSED [90 deg]" : "OPEN [0 deg]"));
        Serial.printf("Buzzer:         %s\n", (actuators.isBuzzerSilenced() ? "MUTED" : "ACTIVE"));
        Serial.printf("WiFi Status:    %s\n", networkMgr.isConnected() ? "CONNECTED" : "OFFLINE");
        Serial.printf("Buffered Pkts:  %u / %u\n", (unsigned int)networkMgr.getBufferedCount(), (unsigned int)OFFLINE_BUFFER_CAPACITY);
        Serial.println(F("=================================================\n"));
    } else if (strcmp(cmd, "mock on") == 0) {
        isMockModeActive = true;
        activeRadar = &mockRadar;
        Serial.println(F("[CLI] Switched radar input to SYNTHETIC MOCK model."));
    } else if (strcmp(cmd, "mock off") == 0) {
        isMockModeActive = false;
        activeRadar = &realRadar;
        Serial.println(F("[CLI] Switched radar input to PHYSICAL HLK-LD2450 UART."));
    } else if (strcmp(cmd, "force red") == 0) {
        stateMachine.forceState(CrowdState::RED, "Serial CLI override: 'force red'");
    } else if (strcmp(cmd, "force green") == 0) {
        stateMachine.forceState(CrowdState::GREEN, "Serial CLI override: 'force green'");
    } else if (strcmp(cmd, "gate open") == 0) {
        actuators.manualSetGate(GatePosition::OPEN, "Serial CLI command");
    } else if (strcmp(cmd, "gate close") == 0) {
        actuators.manualSetGate(GatePosition::CLOSED, "Serial CLI command");
    } else if (strcmp(cmd, "help") == 0) {
        Serial.println(F("Available commands: status, mock on, mock off, force red, force green, gate open, gate close"));
    } else {
        Serial.printf("Unknown command: '%s'. Type 'help' for commands.\n", cmd);
    }
}

// ============================================================================
// ARDUINO SETUP
// ============================================================================
void setup() {
    Serial.begin(115200);
    delay(200);

    Serial.println(F("\n========================================================"));
    Serial.println(F("   CROWD PRESSURE RADAR - EMBEDDED NODE INITIALIZING   "));
    Serial.println(F("   Privacy-First mmWave Queue Early Warning System     "));
    Serial.println(F("========================================================"));

    // Configure Hardware Pin Modes
    pinMode(PIN_MOCK_TOGGLE, INPUT_PULLUP);
    pinMode(PIN_IR_ENTRY, INPUT_PULLUP);
    pinMode(PIN_IR_EXIT, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_IR_ENTRY), isrEntryBeam, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_IR_EXIT), isrExitBeam, FALLING);

    // Read hardware mock mode toggle switch: LOW = Mock, HIGH = Physical
    bool hwPinMockRequested = (digitalRead(PIN_MOCK_TOGGLE) == LOW);
    isMockModeActive = hwPinMockRequested || DEFAULT_MOCK_MODE;

    // Initialize Radar Drivers
    realRadar.begin();
    mockRadar.begin();
    activeRadar = isMockModeActive ? static_cast<IRadar*>(&mockRadar) : static_cast<IRadar*>(&realRadar);
    Serial.printf("[SYS] Selected Initial Radar: %s\n", activeRadar->getDriverName());

    // Initialize Subsystems
    displayMgr.begin();
    actuators.begin();
    stateMachine.begin();
    networkMgr.begin(handleServerCommand);

    Serial.println(F("[SYS] System ready. Non-blocking event loop starting.\n"));
}

// ============================================================================
// NON-BLOCKING MAIN EVENT LOOP
// ============================================================================
void loop() {
    uint32_t now = millis();

    // 1. Process Serial CLI inputs
    while (Serial.available() > 0) {
        char ch = static_cast<char>(Serial.read());
        if (ch == '\r' || ch == '\n') {
            if (serialCmdIndex > 0) {
                serialCmdBuffer[serialCmdIndex] = '\0';
                processSerialCommand(serialCmdBuffer);
                serialCmdIndex = 0;
            }
        } else if (serialCmdIndex < sizeof(serialCmdBuffer) - 1) {
            serialCmdBuffer[serialCmdIndex++] = ch;
        }
    }

    // 2. Poll active radar sensor
    bool newRadarData = activeRadar->update();

    // 3. Run Analytics & State Machine (at 5 Hz or whenever new radar frame arrives)
    if (newRadarData || (now - lastAnalyticsRunMs >= INTERVAL_ANALYTICS_MS)) {
        lastAnalyticsRunMs = now;

        const RadarTarget* targets = activeRadar->getTargets();
        uint8_t count = activeRadar->getTargetCount();

        ZoneMetrics metrics = analytics.process(targets, count);

        // Update State Machine with hysteresis
        StateTransitionEvent transition = stateMachine.update(metrics.riskScore, now);
        if (transition.hasEvent) {
            actuators.handleStateTransition(transition);
        }
    }

    // 4. Update Actuators (LED blinking, buzzer patterns, servo release timer)
    uint32_t greenDuration = stateMachine.getContinuousGreenDuration(now);
    actuators.update(stateMachine.getCurrentState(), now, greenDuration);

    // 5. Update OLED Screen (page rotation, status rendering)
    displayMgr.update(
        stateMachine.getCurrentState(),
        analytics.getLastMetrics(),
        irEntryCount,
        irExitCount,
        actuators.getGatePosition(),
        networkMgr.isConnected(),
        activeRadar->getDriverName(),
        now
    );

    // 6. Periodic Telemetry Queuing (1 Hz)
    if (now - lastTelemetryQueueMs >= INTERVAL_TELEMETRY_MS) {
        lastTelemetryQueueMs = now;
        CrowdState st = stateMachine.getCurrentState();
        const char* alertMsg = (st == CrowdState::RED) ? "RED_CRITICAL_SURGE" :
                               (st == CrowdState::AMBER ? "AMBER_CONGESTION" : "FLOW_NORMAL");

        networkMgr.queueTelemetry(
            analytics.getLastMetrics(),
            st,
            irEntryCount,
            irExitCount,
            alertMsg
        );
    }

    // 7. Network Manager Update (WiFi check, telemetry POST, command polling)
    networkMgr.update(now);
}

