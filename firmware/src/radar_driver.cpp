#include "radar_driver.h"

HLKLD2450Driver::HLKLD2450Driver(HardwareSerial& serialPort, int rxPin, int txPin, uint32_t baudRate)
    : radarSerial(serialPort),
      rxPin(rxPin),
      txPin(txPin),
      baudRate(baudRate),
      activeTargetCount(0),
      validFrames(0),
      droppedFrames(0),
      rxBufferIndex(0) {
    for (int i = 0; i < 3; i++) {
        targets[i] = {0, 0, 0, 0, false};
    }
}

void HLKLD2450Driver::begin() {
    radarSerial.begin(baudRate, SERIAL_8N1, rxPin, txPin);
    rxBufferIndex = 0;
    validFrames = 0;
    droppedFrames = 0;
    activeTargetCount = 0;
    Serial.printf("[RADAR] Initialized HLK-LD2450 on RX=%d, TX=%d @ %u baud\n", rxPin, txPin, baudRate);
}

bool HLKLD2450Driver::parseFrame(const uint8_t* frameData) {
    // Frame format per HLK-LD2450 protocol specification:
    // Header (4 bytes):  0xAA 0xFF 0x03 0x00
    // Target 1 (8 bytes): X_L, X_H, Y_L, Y_H, Speed_L, Speed_H, DistRes_L, DistRes_H
    // Target 2 (8 bytes): same
    // Target 3 (8 bytes): same
    // Footer (2 bytes):  0x55 0xCC
    // Total: 4 + 24 + 2 = 30 bytes

    if (frameData[0] != 0xAA || frameData[1] != 0xFF ||
        frameData[2] != 0x03 || frameData[3] != 0x00) {
        droppedFrames++;
        return false;
    }

    if (frameData[28] != 0x55 || frameData[29] != 0xCC) {
        droppedFrames++;
        return false;
    }

    uint8_t countedTargets = 0;

    for (int i = 0; i < 3; i++) {
        size_t offset = 4 + (i * 8);

        uint16_t raw_x     = (uint16_t)(frameData[offset]     | (frameData[offset + 1] << 8));
        uint16_t raw_y     = (uint16_t)(frameData[offset + 2] | (frameData[offset + 3] << 8));
        uint16_t raw_speed = (uint16_t)(frameData[offset + 4] | (frameData[offset + 5] << 8));
        uint16_t raw_res   = (uint16_t)(frameData[offset + 6] | (frameData[offset + 7] << 8));

        // verify against the LD2450 datasheet
        // Bit 15 represents the sign bit: 1 = negative, 0 = positive
        // Low 15 bits (0x7FFF) provide the magnitude in millimeters
        int16_t x = (raw_x & 0x8000) ? -(int16_t)(raw_x & 0x7FFF) : (int16_t)(raw_x & 0x7FFF);

        // verify against the LD2450 datasheet
        // For Y-distance, distance is positive forward; bit 15 indicates sign
        int16_t y = (raw_y & 0x8000) ? -(int16_t)(raw_y & 0x7FFF) : (int16_t)(raw_y & 0x7FFF);

        // Radial speed: bit 15 indicates direction (away vs towards sensor)
        int16_t speed = (raw_speed & 0x8000) ? -(int16_t)(raw_speed & 0x7FFF) : (int16_t)(raw_speed & 0x7FFF);

        targets[i].x = x;
        targets[i].y = y;
        targets[i].speed = speed;
        targets[i].resolution = raw_res;

        // A target is considered valid if it has non-zero resolution and sensible positive distance
        if (raw_res > 0 && y > 50 && y <= 6500) {
            targets[i].valid = true;
            countedTargets++;
        } else {
            targets[i].valid = false;
        }
    }

    activeTargetCount = countedTargets;
    validFrames++;
    return true;
}

bool HLKLD2450Driver::update() {
    bool newFrameParsed = false;

    while (radarSerial.available() > 0) {
        uint8_t byteIn = radarSerial.read();

        // Search for synchronization header 0xAA 0xFF 0x03 0x00
        if (rxBufferIndex == 0) {
            if (byteIn == 0xAA) {
                rxBuffer[rxBufferIndex++] = byteIn;
            }
        } else if (rxBufferIndex == 1) {
            if (byteIn == 0xFF) {
                rxBuffer[rxBufferIndex++] = byteIn;
            } else {
                rxBufferIndex = 0;
            }
        } else if (rxBufferIndex == 2) {
            if (byteIn == 0x03) {
                rxBuffer[rxBufferIndex++] = byteIn;
            } else {
                rxBufferIndex = 0;
            }
        } else if (rxBufferIndex == 3) {
            if (byteIn == 0x00) {
                rxBuffer[rxBufferIndex++] = byteIn;
            } else {
                rxBufferIndex = 0;
            }
        } else {
            // Header matched, collect remaining payload
            rxBuffer[rxBufferIndex++] = byteIn;

            if (rxBufferIndex == FRAME_LENGTH) {
                if (parseFrame(rxBuffer)) {
                    newFrameParsed = true;
                }
                rxBufferIndex = 0;
            }
        }

        // Buffer overrun protection
        if (rxBufferIndex >= sizeof(rxBuffer)) {
            rxBufferIndex = 0;
            droppedFrames++;
        }
    }

    return newFrameParsed;
}

