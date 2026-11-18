#pragma once

#include <Arduino.h>
#include "radar_driver.h"
#include "config.h"

/**
 * Calculated Crowd Risk Metrics for a Zone
 */
struct ZoneMetrics {
    float density;          // Targets per square meter (ppl / m²)
    float avgSpeed;         // Mean target speed in m/s
    float stopGoIndex;      // Standard deviation of speed over sliding window (m/s)
    uint16_t clusterPairs;  // Number of target pairs within compression distance (< 0.5m)
    float riskScore;        // Composite Risk Score (0.0 - 100.0)
    uint8_t targetCount;    // Number of active targets
};

/**
 * Pure analytics functions for crowd analysis
 */
namespace CrowdAnalytics {

    // Computes crowd density: targets / zoneAreaSqM
    float computeDensity(uint8_t activeTargets, float zoneAreaSqM);

    // Computes average speed of active targets in m/s (converts from cm/s)
    float computeAverageSpeed(const RadarTarget* targets, uint8_t maxTargets);

    // Computes clustering: count of target pairs within distanceThresholdMm (default 500 mm)
    uint16_t computeClustering(const RadarTarget* targets, uint8_t maxTargets, float distanceThresholdMm = 500.0f);

    // Updates sliding speed window and returns standard deviation of speed
    float updateStopAndGoIndex(float currentSpeedMps, float* windowBuffer, uint8_t windowCapacity,
                               uint8_t& currentSampleCount, uint8_t& writeIndex);

    // Combines metrics into 0-100 score using calibrated weights
    float computeCompositeRiskScore(float density, float avgSpeedMps, float stopGoStdDev,
                                    uint16_t clusterPairs, float zoneAreaSqM = DEFAULT_ZONE_AREA_SQM);
}

/**
 * Stateful analytics manager handling historical window buffers
 */
class AnalyticsEngine {
public:
    AnalyticsEngine(float zoneAreaSqM = DEFAULT_ZONE_AREA_SQM);

    void reset();
    ZoneMetrics process(const RadarTarget* targets, uint8_t maxTargets);
    const ZoneMetrics& getLastMetrics() const { return lastMetrics; }

private:
    float zoneAreaSqM;
    float speedWindow[SPEED_WINDOW_SIZE];
    uint8_t speedSampleCount;
    uint8_t speedWriteIndex;
    ZoneMetrics lastMetrics;
};

