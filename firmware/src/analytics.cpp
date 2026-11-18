#include "analytics.h"
#include <cmath>
#include <algorithm>

namespace CrowdAnalytics {

float computeDensity(uint8_t activeTargets, float zoneAreaSqM) {
    if (zoneAreaSqM <= 0.01f) {
        return 0.0f;
    }
    return static_cast<float>(activeTargets) / zoneAreaSqM;
}

float computeAverageSpeed(const RadarTarget* targets, uint8_t maxTargets) {
    if (targets == nullptr || maxTargets == 0) {
        return 0.0f;
    }

    float speedSum = 0.0f;
    uint8_t count = 0;

    for (uint8_t i = 0; i < maxTargets; i++) {
        if (targets[i].valid) {
            // Speed reported in cm/s; convert to m/s magnitude
            float speedMps = std::abs(static_cast<float>(targets[i].speed)) / 100.0f;
            speedSum += speedMps;
            count++;
        }
    }

    if (count == 0) {
        return 0.0f;
    }
    return speedSum / static_cast<float>(count);
}

uint16_t computeClustering(const RadarTarget* targets, uint8_t maxTargets, float distanceThresholdMm) {
    if (targets == nullptr || maxTargets < 2) {
        return 0;
    }

    uint16_t closePairCount = 0;

    for (uint8_t i = 0; i < maxTargets; i++) {
        if (!targets[i].valid) continue;

        for (uint8_t j = i + 1; j < maxTargets; j++) {
            if (!targets[j].valid) continue;

            float dx = static_cast<float>(targets[i].x - targets[j].x);
            float dy = static_cast<float>(targets[i].y - targets[j].y);
            float distanceMm = std::sqrt((dx * dx) + (dy * dy));

            if (distanceMm < distanceThresholdMm) {
                closePairCount++;
            }
        }
    }

    return closePairCount;
}

float updateStopAndGoIndex(float currentSpeedMps, float* windowBuffer, uint8_t windowCapacity,
                           uint8_t& currentSampleCount, uint8_t& writeIndex) {
    if (windowBuffer == nullptr || windowCapacity == 0) {
        return 0.0f;
    }

    windowBuffer[writeIndex] = currentSpeedMps;
    writeIndex = (writeIndex + 1) % windowCapacity;

    if (currentSampleCount < windowCapacity) {
        currentSampleCount++;
    }

    if (currentSampleCount < 2) {
        return 0.0f;
    }

    // Compute arithmetic mean
    float sum = 0.0f;
    for (uint8_t i = 0; i < currentSampleCount; i++) {
        sum += windowBuffer[i];
    }
    float mean = sum / static_cast<float>(currentSampleCount);

    // Compute standard deviation
    float varianceSum = 0.0f;
    for (uint8_t i = 0; i < currentSampleCount; i++) {
        float diff = windowBuffer[i] - mean;
        varianceSum += (diff * diff);
    }

    return std::sqrt(varianceSum / static_cast<float>(currentSampleCount));
}

float computeCompositeRiskScore(float density, float avgSpeedMps, float stopGoStdDev,
                                uint16_t clusterPairs, float zoneAreaSqM) {
    // 1. Density Component (0 - 100)
    // Scaled against DENSITY_CRITICAL_THRESHOLD (e.g. 4.0 ppl/m²)
    float s_density = std::min(100.0f, (density / DENSITY_CRITICAL_THRESHOLD) * 100.0f);

    // 2. Low Speed Component (0 - 100)
    // Speed >= SPEED_NORMAL_MPS (1.3 m/s) -> 0% risk
    // Speed <= SPEED_STALL_MPS (0.15 m/s) -> 100% risk
    float s_low_speed = 0.0f;
    if (avgSpeedMps <= SPEED_STALL_MPS) {
        s_low_speed = 100.0f;
    } else if (avgSpeedMps < SPEED_NORMAL_MPS) {
        float span = SPEED_NORMAL_MPS - SPEED_STALL_MPS;
        s_low_speed = ((SPEED_NORMAL_MPS - avgSpeedMps) / span) * 100.0f;
        s_low_speed = std::min(100.0f, std::max(0.0f, s_low_speed));
    }

    // 3. Stop-and-Go Turbulence Component (0 - 100)
    // High standard deviation indicates shockwave stops & starts
    float s_stop_go = std::min(100.0f, (stopGoStdDev / STOP_GO_MAX_STDDEV) * 100.0f);

    // 4. Clustering / Compression Component (0 - 100)
    // Each target pair closer than 0.5 m adds 50%
    float s_clustering = std::min(100.0f, static_cast<float>(clusterPairs) * 50.0f);

    // Weighted composite combination (weights from config.h: 0.4, 0.3, 0.2, 0.1)
    float compositeScore = (WEIGHT_DENSITY     * s_density) +
                           (WEIGHT_LOW_SPEED   * s_low_speed) +
                           (WEIGHT_STOP_AND_GO * s_stop_go) +
                           (WEIGHT_CLUSTERING  * s_clustering);

    return std::min(100.0f, std::max(0.0f, compositeScore));
}

} // namespace CrowdAnalytics

AnalyticsEngine::AnalyticsEngine(float zoneAreaSqM)
    : zoneAreaSqM(zoneAreaSqM),
      speedSampleCount(0),
      speedWriteIndex(0) {
    reset();
}

void AnalyticsEngine::reset() {
    speedSampleCount = 0;
    speedWriteIndex = 0;
    for (uint8_t i = 0; i < SPEED_WINDOW_SIZE; i++) {
        speedWindow[i] = 0.0f;
    }
    lastMetrics = {0.0f, 0.0f, 0.0f, 0, 0.0f, 0};
}

ZoneMetrics AnalyticsEngine::process(const RadarTarget* targets, uint8_t maxTargets) {
    uint8_t activeCount = 0;
    if (targets != nullptr) {
        for (uint8_t i = 0; i < maxTargets; i++) {
            if (targets[i].valid) {
                activeCount++;
            }
        }
    }

    lastMetrics.targetCount = activeCount;
    lastMetrics.density = CrowdAnalytics::computeDensity(activeCount, zoneAreaSqM);
    lastMetrics.avgSpeed = CrowdAnalytics::computeAverageSpeed(targets, maxTargets);
    lastMetrics.clusterPairs = CrowdAnalytics::computeClustering(targets, maxTargets, CLUSTERING_DISTANCE_M * 1000.0f);

    lastMetrics.stopGoIndex = CrowdAnalytics::updateStopAndGoIndex(
        lastMetrics.avgSpeed,
        speedWindow,
        SPEED_WINDOW_SIZE,
        speedSampleCount,
        speedWriteIndex
    );

    lastMetrics.riskScore = CrowdAnalytics::computeCompositeRiskScore(
        lastMetrics.density,
        lastMetrics.avgSpeed,
        lastMetrics.stopGoIndex,
        lastMetrics.clusterPairs,
        zoneAreaSqM
    );

    return lastMetrics;
}

