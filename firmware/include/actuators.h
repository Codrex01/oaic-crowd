#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "pins.h"
#include "config.h"
#include "state_machine.h"

enum class GatePosition {
    OPEN = 0,
    CLOSED = 90
};

class ActuatorController {
public:
    ActuatorController();

    void begin();
    void update(CrowdState state, uint32_t now, uint32_t continuousGreenDuration);
    void handleStateTransition(const StateTransitionEvent& event);

    // Manual controls / server command handlers
    void manualSetGate(GatePosition position, const char* reason);
    void silenceBuzzer(bool silence);
    bool isBuzzerSilenced() const { return buzzerMuted; }
    GatePosition getGatePosition() const { return currentGatePos; }

private:
    Servo gateServo;
    GatePosition currentGatePos;
    bool servoAttached;
    bool manualGateOverride;

    bool buzzerMuted;
    uint32_t singleBeepUntilMs;
    uint32_t lastLedBlinkToggleMs;
    bool redBlinkState;

    void setGateAngle(int angle, const char* reason);
    void updateLeds(CrowdState state, uint32_t now);
    void updateBuzzer(CrowdState state, uint32_t now);
};

