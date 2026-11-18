#pragma once

#include <Arduino.h>
#include "config.h"

enum class CrowdState {
    GREEN,
    AMBER,
    RED
};

struct StateTransitionEvent {
    CrowdState fromState;
    CrowdState toState;
    float triggerScore;
    uint32_t timestampMs;
    const char* reason;
    bool hasEvent;
};

class CrowdStateMachine {
public:
    CrowdStateMachine();

    void begin();
    StateTransitionEvent update(float currentScore, uint32_t now);
    void forceState(CrowdState newState, const char* reason);
    void reset();

    CrowdState getCurrentState() const { return currentState; }
    const char* getCurrentStateName() const { return getStateName(currentState); }
    uint32_t getTimeInCurrentState(uint32_t now) const { return now - stateEnteredMs; }
    uint32_t getContinuousGreenDuration(uint32_t now) const;

    static const char* getStateName(CrowdState state);

private:
    CrowdState currentState;
    uint32_t stateEnteredMs;
    uint32_t continuousGreenStartMs;

    // Escalation candidates
    uint32_t redCandidateStartMs;
    uint32_t amberCandidateStartMs;

    // De-escalation candidates
    uint32_t deescalateCandidateStartMs;

    StateTransitionEvent transitionTo(CrowdState newState, float score, uint32_t now, const char* reason);
};

