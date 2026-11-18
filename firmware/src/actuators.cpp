#include "actuators.h"

ActuatorController::ActuatorController()
    : currentGatePos(GatePosition::OPEN),
      servoAttached(false),
      manualGateOverride(false),
      buzzerMuted(false),
      singleBeepUntilMs(0),
      lastLedBlinkToggleMs(0),
      redBlinkState(false) {}

void ActuatorController::begin() {
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_YELLOW, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_BUZZER, LOW);

    // Initialize SG90 servo via ESP32Servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    gateServo.setPeriodHertz(50); // Standard 50Hz servo
    gateServo.attach(PIN_SERVO, 500, 2400); // 500us to 2400us pulse width
    servoAttached = true;

    setGateAngle(SERVO_OPEN_ANGLE, "System Boot / Initial Open State");
    Serial.println(F("[ACTUATORS] Hardware initialized (LEDs, Buzzer, SG90 Servo on GPIO13)."));
}

void ActuatorController::setGateAngle(int angle, const char* reason) {
    if (servoAttached) {
        gateServo.write(angle);
        currentGatePos = (angle >= 45) ? GatePosition::CLOSED : GatePosition::OPEN;
        Serial.printf("[SERVO GATE] Position -> %d deg (%s) | Reason: %s\n",
                      angle, (currentGatePos == GatePosition::CLOSED ? "CLOSED" : "OPEN"), reason);
    }
}

void ActuatorController::handleStateTransition(const StateTransitionEvent& event) {
    if (!event.hasEvent) return;

    // Reset manual override on state escalation
    if (event.toState == CrowdState::RED) {
        manualGateOverride = false;
        setGateAngle(SERVO_CLOSED_ANGLE, "Automatic RED State Lockout - Emergency Crowd Diversion");
    } else if (event.toState == CrowdState::AMBER) {
        // Single warning beep on entering AMBER (200 ms)
        singleBeepUntilMs = millis() + 200;
    }
}

void ActuatorController::manualSetGate(GatePosition position, const char* reason) {
    manualGateOverride = true;
    setGateAngle(static_cast<int>(position), reason);
}

void ActuatorController::silenceBuzzer(bool silence) {
    buzzerMuted = silence;
    if (buzzerMuted) {
        digitalWrite(PIN_BUZZER, LOW);
        Serial.println(F("[BUZZER] Alert sound muted by operator command."));
    } else {
        Serial.println(F("[BUZZER] Alert sound unmuted."));
    }
}

void ActuatorController::updateLeds(CrowdState state, uint32_t now) {
    switch (state) {
        case CrowdState::GREEN:
            digitalWrite(PIN_LED_GREEN, HIGH);
            digitalWrite(PIN_LED_YELLOW, LOW);
            digitalWrite(PIN_LED_RED, LOW);
            break;

        case CrowdState::AMBER:
            digitalWrite(PIN_LED_GREEN, LOW);
            digitalWrite(PIN_LED_YELLOW, HIGH);
            digitalWrite(PIN_LED_RED, LOW);
            break;

        case CrowdState::RED:
            digitalWrite(PIN_LED_GREEN, LOW);
            digitalWrite(PIN_LED_YELLOW, LOW);

            // 2 Hz flashing = 250 ms ON, 250 ms OFF
            if (now - lastLedBlinkToggleMs >= 250) {
                lastLedBlinkToggleMs = now;
                redBlinkState = !redBlinkState;
                digitalWrite(PIN_LED_RED, redBlinkState ? HIGH : LOW);
            }
            break;
    }
}

void ActuatorController::updateBuzzer(CrowdState state, uint32_t now) {
    if (buzzerMuted) {
        digitalWrite(PIN_BUZZER, LOW);
        return;
    }

    if (state == CrowdState::GREEN) {
        digitalWrite(PIN_BUZZER, LOW);
    } else if (state == CrowdState::AMBER) {
        // Single beep pulse on transition
        if (now < singleBeepUntilMs) {
            digitalWrite(PIN_BUZZER, HIGH);
        } else {
            digitalWrite(PIN_BUZZER, LOW);
        }
    } else if (state == CrowdState::RED) {
        // High urgency intermittent beep: 250ms ON, 250ms OFF
        bool buzzerPattern = (now / 250) % 2 == 0;
        digitalWrite(PIN_BUZZER, buzzerPattern ? HIGH : LOW);
    }
}

void ActuatorController::update(CrowdState state, uint32_t now, uint32_t continuousGreenDuration) {
    updateLeds(state, now);
    updateBuzzer(state, now);

    // Gate barrier logic:
    // If gate is CLOSED and not under manual override, check for 30 seconds of continuous GREEN
    if (!manualGateOverride && currentGatePos == GatePosition::CLOSED) {
        if (state == CrowdState::GREEN && continuousGreenDuration >= SERVO_RESET_DELAY_MS) {
            setGateAngle(SERVO_OPEN_ANGLE, "Continuous GREEN duration exceeded 30s - Flow safe to resume");
        }
    }
}

