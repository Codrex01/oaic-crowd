#pragma once

#include <Arduino.h>

/**
 * ============================================================================
 * CROWD PRESSURE RADAR - HARDWARE PIN MAPPING
 * Target: ESP32 DevKit C V4 (30-pin, CP2102)
 * ============================================================================
 */

// HLK-LD2450 24GHz mmWave Radar (UART2)
constexpr int PIN_RADAR_RX = 16;  // ESP32 RX2 connected to LD2450 TX
constexpr int PIN_RADAR_TX = 17;  // ESP32 TX2 connected to LD2450 RX

// I2C 0.96" SSD1306 OLED (128x64)
constexpr int PIN_OLED_SDA = 21;
constexpr int PIN_OLED_SCL = 22;

// Traffic / Alert Indicator LEDs (Active HIGH through 220 ohm resistors)
constexpr int PIN_LED_GREEN  = 25;
constexpr int PIN_LED_YELLOW = 26;
constexpr int PIN_LED_RED    = 27;

// 5V Active Buzzer Module (Active HIGH or via NPN transistor)
constexpr int PIN_BUZZER = 14;

// SG90 Micro Servo (PWM Gate Barrier)
constexpr int PIN_SERVO = 13;

// IR Break-Beam Sensors (People Counting, Active LOW with internal pull-up)
constexpr int PIN_IR_ENTRY = 32;  // Beam A: Entry
constexpr int PIN_IR_EXIT  = 33;  // Beam B: Exit

// Hardware Mode Selector (Active LOW with internal pull-up)
// LOW  = Synthetic Mock Radar (default for Wokwi simulation)
// HIGH = Physical HLK-LD2450 UART Radar
constexpr int PIN_MOCK_TOGGLE = 4;

// Optional expansion pins (reserved for modular upgrade, not required for demo)
// I2C VL53L1X ToF: Shares PIN_OLED_SDA (21) & PIN_OLED_SCL (22)
// SX1278 SPI LoRa Module:
// SCK: 18, MISO: 19, MOSI: 23, NSS: 5, RST: 15, DIO0: 2

