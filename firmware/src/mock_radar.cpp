#include "mock_radar.h"
#include <cmath>

MockRadar::MockRadar()
    : activeTargetCount(0),
      validFrames(0),
      lastUpdateMs(0),
      phaseStartMs(0),
      currentPhase(MockScenarioPhase::FREE_FLOW) {
    for (int i = 0; i < 3; i++) {
        targets[i] = {0, 0, 0, 0, false};
    }
}

void MockRadar::begin() {
    activeTargetCount = 1;
    validFrames = 0;
    lastUpdateMs = millis();
    phaseStartMs = millis();
    currentPhase = MockScenarioPhase::FREE_FLOW;

    // Initial starting positions
    targets[0] = { -400, 4200, -125, 120, true };
    targets[1] = {  300, 5100, -135, 120, false };
    targets[2] = {    0, 5800, -110, 120, false };

    Serial.println(F("[MOCK] Initialized Synthetic Crowd Radar Model."));
}

const char* MockRadar::getPhaseName() const {
    switch (currentPhase) {
        case MockScenarioPhase::FREE_FLOW:   return "FREE_FLOW";
        case MockScenarioPhase::BUILDUP:     return "BUILDUP";
        case MockScenarioPhase::STALL_EVENT: return "STALL_CRUSH";
        case MockScenarioPhase::RECOVERY:    return "RECOVERY";
        default:                             return "UNKNOWN";
    }
}

void MockRadar::updateScenario(uint32_t now) {
    uint32_t elapsedInPhase = now - phaseStartMs;

    // Timeline:
    // 0s - 14s: Free flow (low density, normal walking speed ~1.2 m/s)
    // 14s - 26s: Buildup (3 targets, speed slows to ~0.7 m/s)
    // 26s - 42s: Stall event (speed plummets to ~0.08 m/s, targets compress within 35 cm)
    // 42s - 65s: Recovery (gate closure effect, flow disperses back to normal)
    if (currentPhase == MockScenarioPhase::FREE_FLOW && elapsedInPhase > 14000) {
        currentPhase = MockScenarioPhase::BUILDUP;
        phaseStartMs = now;
        Serial.println(F("[MOCK SCENARIO] -> Transition to BUILDUP (Queue condensing)"));
    } else if (currentPhase == MockScenarioPhase::BUILDUP && elapsedInPhase > 12000) {
        currentPhase = MockScenarioPhase::STALL_EVENT;
        phaseStartMs = now;
        Serial.println(F("[MOCK SCENARIO] -> Transition to STALL_CRUSH (Bottleneck formed, movement stopped)"));
    } else if (currentPhase == MockScenarioPhase::STALL_EVENT && elapsedInPhase > 16000) {
        currentPhase = MockScenarioPhase::RECOVERY;
        phaseStartMs = now;
        Serial.println(F("[MOCK SCENARIO] -> Transition to RECOVERY (Gate deployed, pressure easing)"));
    } else if (currentPhase == MockScenarioPhase::RECOVERY && elapsedInPhase > 20000) {
        currentPhase = MockScenarioPhase::FREE_FLOW;
        phaseStartMs = now;
        Serial.println(F("[MOCK SCENARIO] -> Transition to FREE_FLOW (Normal flow restored)"));
    }

    // Update targets based on current phase
    switch (currentPhase) {
        case MockScenarioPhase::FREE_FLOW: {
            activeTargetCount = 2;

            // Target 0: Walking steadily towards gate (decreasing Y)
            targets[0].valid = true;
            targets[0].x = -350 + (int16_t)(sin(now / 800.0) * 80.0);
            targets[0].speed = -120 + (int16_t)(sin(now / 500.0) * 15.0); // ~ -1.2 m/s
            targets[0].y -= (int16_t)(std::abs(targets[0].speed) * 0.1);
            if (targets[0].y < 500) targets[0].y = 5200;
            targets[0].resolution = 120;

            // Target 1: Another walking person staggered
            targets[1].valid = true;
            targets[1].x = 420 + (int16_t)(cos(now / 900.0) * 70.0);
            targets[1].speed = -130 + (int16_t)(cos(now / 600.0) * 10.0); // ~ -1.3 m/s
            targets[1].y -= (int16_t)(std::abs(targets[1].speed) * 0.1);
            if (targets[1].y < 500) targets[1].y = 5800;
            targets[1].resolution = 130;

            targets[2].valid = false;
            break;
        }

        case MockScenarioPhase::BUILDUP: {
            activeTargetCount = 3;

            // Target 0: Slowing down
            targets[0].valid = true;
            targets[0].x = -200 + (int16_t)(sin(now / 600.0) * 40.0);
            targets[0].speed = -65 + (int16_t)(sin(now / 400.0) * 20.0); // ~ -0.65 m/s
            targets[0].y -= (int16_t)(std::abs(targets[0].speed) * 0.1);
            if (targets[0].y < 1200) targets[0].y = 4500;
            targets[0].resolution = 110;

            // Target 1: Closing in behind
            targets[1].valid = true;
            targets[1].x = 150 + (int16_t)(cos(now / 700.0) * 40.0);
            targets[1].speed = -55 + (int16_t)(cos(now / 450.0) * 25.0); // ~ -0.55 m/s
            targets[1].y = targets[0].y + 900; // 0.9 m behind
            targets[1].resolution = 115;

            // Target 2: Entering queue
            targets[2].valid = true;
            targets[2].x = -50 + (int16_t)(sin(now / 900.0) * 30.0);
            targets[2].speed = -70 + (int16_t)(sin(now / 500.0) * 20.0);
            targets[2].y = targets[1].y + 1100;
            targets[2].resolution = 100;
            break;
        }

        case MockScenarioPhase::STALL_EVENT: {
            // High compression stall: 3 people crowded into tight cluster < 0.45m apart
            activeTargetCount = 3;

            // Near-halt speeds with small chaotic stop-and-go jerks
            int16_t jitterSpeed = (int16_t)(sin(now / 200.0) * 15.0); // oscillations

            // Target 0: Front of bottleneck (stalled at Y=2000 mm)
            targets[0].valid = true;
            targets[0].x = 50 + (int16_t)(sin(now / 350.0) * 25.0);
            targets[0].y = 2000 + (int16_t)(cos(now / 300.0) * 20.0);
            targets[0].speed = -8 + jitterSpeed; // crawl ~ 0.08 m/s
            targets[0].resolution = 150;

            // Target 1: Pressed closely right next to Target 0 (distance ~ 320 mm)
            targets[1].valid = true;
            targets[1].x = targets[0].x + 300; // 30 cm apart in X
            targets[1].y = targets[0].y + 120; // 12 cm offset in Y
            targets[1].speed = -5 - jitterSpeed;
            targets[1].resolution = 145;

            // Target 2: Pushing directly from behind (distance ~ 380 mm to Target 0)
            targets[2].valid = true;
            targets[2].x = targets[0].x - 80;
            targets[2].y = targets[0].y + 370; // 37 cm behind Target 0
            targets[2].speed = -12 + (jitterSpeed / 2);
            targets[2].resolution = 140;
            break;
        }

        case MockScenarioPhase::RECOVERY: {
            activeTargetCount = 2;

            // Flow moving again, targets separating
            targets[0].valid = true;
            targets[0].x = -300;
            targets[0].speed = -105 + (int16_t)(sin(now / 500.0) * 10.0); // ~ 1.05 m/s
            targets[0].y -= (int16_t)(std::abs(targets[0].speed) * 0.1);
            if (targets[0].y < 500) targets[0].y = 4800;
            targets[0].resolution = 120;

            targets[1].valid = true;
            targets[1].x = 350;
            targets[1].speed = -115 + (int16_t)(cos(now / 500.0) * 10.0);
            targets[1].y = targets[0].y + 1800; // 1.8 m separation
            targets[1].resolution = 120;

            targets[2].valid = false;
            break;
        }
    }
}

bool MockRadar::update() {
    uint32_t now = millis();
    // Simulate ~10Hz LD2450 reporting cycle (100ms)
    if (now - lastUpdateMs >= 100) {
        lastUpdateMs = now;
        updateScenario(now);
        validFrames++;
        return true;
    }
    return false;
}

