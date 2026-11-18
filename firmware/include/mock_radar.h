#pragma once

#include "radar_driver.h"

enum class MockScenarioPhase {
    FREE_FLOW,          // Normal walking, dispersed targets
    BUILDUP,            // Increased density, slowing movement
    STALL_EVENT,        // Severe crowd stall, bunching (<0.5m), near-zero speed
    RECOVERY            // Gate deployed, crowd clearing, flow restoring
};

class MockRadar : public IRadar {
public:
    MockRadar();
    void begin() override;
    bool update() override;
    uint8_t getTargetCount() const override { return activeTargetCount; }
    const RadarTarget* getTargets() const override { return targets; }
    uint32_t getValidFrames() const override { return validFrames; }
    uint32_t getDroppedFrames() const override { return 0; }
    const char* getDriverName() const override { return "Synthetic Mock (Wokwi/Demo)"; }

    MockScenarioPhase getCurrentPhase() const { return currentPhase; }
    const char* getPhaseName() const;

private:
    RadarTarget targets[3];
    uint8_t activeTargetCount;
    uint32_t validFrames;
    uint32_t lastUpdateMs;
    uint32_t phaseStartMs;
    MockScenarioPhase currentPhase;

    void updateScenario(uint32_t now);
};

