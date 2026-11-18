#pragma once

#include <Arduino.h>

/**
 * Radar target tracked by HLK-LD2450 or Synthetic Generator
 */
struct RadarTarget {
    int16_t x;          // X position in mm (-3000 to +3000)
    int16_t y;          // Y position in mm (0 to +6000)
    int16_t speed;      // Radial speed in cm/s (positive = moving away, negative = moving towards)
    uint16_t resolution;// Distance resolution in mm
    bool valid;         // Flag indicating whether target is actively tracked
};

/**
 * Common pure-virtual interface for radar data sources
 */
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

/**
 * Physical HLK-LD2450 UART Driver
 */
class HLKLD2450Driver : public IRadar {
public:
    HLKLD2450Driver(HardwareSerial& serialPort, int rxPin, int txPin, uint32_t baudRate);
    void begin() override;
    bool update() override;
    uint8_t getTargetCount() const override { return activeTargetCount; }
    const RadarTarget* getTargets() const override { return targets; }
    uint32_t getValidFrames() const override { return validFrames; }
    uint32_t getDroppedFrames() const override { return droppedFrames; }
    const char* getDriverName() const override { return "HLK-LD2450 UART"; }

private:
    HardwareSerial& radarSerial;
    int rxPin;
    int txPin;
    uint32_t baudRate;

    RadarTarget targets[3];
    uint8_t activeTargetCount;
    uint32_t validFrames;
    uint32_t droppedFrames;

    static constexpr size_t FRAME_LENGTH = 30;
    uint8_t rxBuffer[64];
    size_t rxBufferIndex;

    bool parseFrame(const uint8_t* frameData);
};

