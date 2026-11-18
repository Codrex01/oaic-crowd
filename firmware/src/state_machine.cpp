#include "state_machine.h"

CrowdStateMachine::CrowdStateMachine()
    : currentState(CrowdState::GREEN),
      stateEnteredMs(0),
      continuousGreenStartMs(0),
      redCandidateStartMs(0),
      amberCandidateStartMs(0),
      deescalateCandidateStartMs(0) {}

void CrowdStateMachine::begin() {
    uint32_t now = millis();
    currentState = CrowdState::GREEN;
    stateEnteredMs = now;
    continuousGreenStartMs = now;
    redCandidateStartMs = 0;
    amberCandidateStartMs = 0;
    deescalateCandidateStartMs = 0;

    Serial.printf("[STATE] Initialized at GREEN [time=%u ms]\n", now);
}

void CrowdStateMachine::reset() {
    begin();
}

const char* CrowdStateMachine::getStateName(CrowdState state) {
    switch (state) {
        case CrowdState::GREEN: return "GREEN";
        case CrowdState::AMBER: return "AMBER";
        case CrowdState::RED:   return "RED";
        default:                return "UNKNOWN";
    }
}

uint32_t CrowdStateMachine::getContinuousGreenDuration(uint32_t now) const {
    if (currentState == CrowdState::GREEN) {
        return now - continuousGreenStartMs;
    }
    return 0;
}

StateTransitionEvent CrowdStateMachine::transitionTo(CrowdState newState, float score, uint32_t now, const char* reason) {
    CrowdState previousState = currentState;
    currentState = newState;
    stateEnteredMs = now;

    if (newState == CrowdState::GREEN) {
        continuousGreenStartMs = now;
    } else {
        continuousGreenStartMs = 0;
    }

    // Reset candidate tracking timers
    redCandidateStartMs = 0;
    amberCandidateStartMs = 0;
    deescalateCandidateStartMs = 0;

    Serial.printf("[STATE TRANSITION] [%u ms] %s -> %s | Score: %.1f | Reason: %s\n",
                  now, getStateName(previousState), getStateName(newState), score, reason);

    return { previousState, newState, score, now, reason, true };
}

StateTransitionEvent CrowdStateMachine::update(float currentScore, uint32_t now) {
    // -------------------------------------------------------------------------
    // 1. Current State: GREEN
    // -------------------------------------------------------------------------
    if (currentState == CrowdState::GREEN) {
        // Direct critical spike to RED (Score >= 75 for 3s)
        if (currentScore >= SCORE_RED_THRESHOLD) {
            if (redCandidateStartMs == 0) {
                redCandidateStartMs = now;
            } else if (now - redCandidateStartMs >= DURATION_ESCALATE_RED_MS) {
                return transitionTo(CrowdState::RED, currentScore, now, "Score >= 75 for >= 3s (Critical surge)");
            }
        } else {
            redCandidateStartMs = 0;
        }

        // Escalation to AMBER (Score >= 50 for 5s)
        if (currentScore >= SCORE_AMBER_THRESHOLD) {
            if (amberCandidateStartMs == 0) {
                amberCandidateStartMs = now;
            } else if (now - amberCandidateStartMs >= DURATION_ESCALATE_AMBER_MS) {
                return transitionTo(CrowdState::AMBER, currentScore, now, "Score >= 50 for >= 5s (Congestion buildup)");
            }
        } else {
            amberCandidateStartMs = 0;
        }
    }
    // -------------------------------------------------------------------------
    // 2. Current State: AMBER
    // -------------------------------------------------------------------------
    else if (currentState == CrowdState::AMBER) {
        // Escalation to RED (Score >= 75 for 3s)
        if (currentScore >= SCORE_RED_THRESHOLD) {
            if (redCandidateStartMs == 0) {
                redCandidateStartMs = now;
            } else if (now - redCandidateStartMs >= DURATION_ESCALATE_RED_MS) {
                return transitionTo(CrowdState::RED, currentScore, now, "Score >= 75 for >= 3s (Stall / severe crowding)");
            }
        } else {
            redCandidateStartMs = 0;
        }

        // De-escalation to GREEN (Score < 50 - 10 = 40 for 10s)
        constexpr float deescalateThreshold = SCORE_AMBER_THRESHOLD - HYSTERESIS_MARGIN;
        if (currentScore < deescalateThreshold) {
            if (deescalateCandidateStartMs == 0) {
                deescalateCandidateStartMs = now;
            } else if (now - deescalateCandidateStartMs >= DURATION_DEESCALATE_MS) {
                return transitionTo(CrowdState::GREEN, currentScore, now, "Score < 40 for >= 10s (Flow cleared)");
            }
        } else {
            deescalateCandidateStartMs = 0;
        }
    }
    // -------------------------------------------------------------------------
    // 3. Current State: RED
    // -------------------------------------------------------------------------
    else if (currentState == CrowdState::RED) {
        // De-escalation to AMBER (Score < 75 - 10 = 65 for 10s)
        constexpr float deescalateThreshold = SCORE_RED_THRESHOLD - HYSTERESIS_MARGIN;
        if (currentScore < deescalateThreshold) {
            if (deescalateCandidateStartMs == 0) {
                deescalateCandidateStartMs = now;
            } else if (now - deescalateCandidateStartMs >= DURATION_DEESCALATE_MS) {
                return transitionTo(CrowdState::AMBER, currentScore, now, "Score < 65 for >= 10s (Pressure easing)");
            }
        } else {
            deescalateCandidateStartMs = 0;
        }
    }

    return { currentState, currentState, currentScore, now, nullptr, false };
}

void CrowdStateMachine::forceState(CrowdState newState, const char* reason) {
    if (newState != currentState) {
        transitionTo(newState, 0.0f, millis(), reason);
    }
}

