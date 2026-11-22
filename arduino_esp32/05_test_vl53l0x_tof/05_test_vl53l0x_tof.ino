/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 05: VL53L0X / TOF200C Laser ToF Sensor
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * PIN CONNECTIONS:
 * - TOF VIN: 3.3V (or 5V if module has onboard 3.3V regulator)
 * - TOF GND: GND
 * - TOF SDA: GPIO 21
 * - TOF SCL: GPIO 22
 * - I2C Address: 0x29 (Standard)
 * 
 * REQUIRED LIBRARY:
 * - "Adafruit_VL53L0X" by Adafruit (Install via Arduino Library Manager)
 * 
 * FUNCTIONALITY:
 * - Emits invisible 940nm VCSEL laser pulses to compute distance with millimeter precision.
 * - Detects physical crowd bottleneck proximity at the corridor gate:
 *   - < 300 mm:  "HIGH COMPRESSION BOTTLENECK"
 *   - 300-800 mm: "PERSON CROSSING GATE"
 *   - > 800 mm:  "CORRIDOR CLEAR"
 * - Includes an automatic I2C Bus Scanner on startup to verify 0x29 connection!
 * =============================================================================
 */

#include <Wire.h>
#include <Adafruit_VL53L0X.h>

#define PIN_TOF_SDA 21
#define PIN_TOF_SCL 22

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
bool tofDetected = false;

void scanI2CBus() {
  Serial.println("[I2C SCANNER] Scanning I2C bus (SDA=21, SCL=22)...");
  byte count = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("  -> Device found at address 0x");
      if (i < 16) Serial.print("0");
      Serial.print(i, HEX);
      if (i == 0x29) Serial.print(" (VL53L0X / TOF200C Distance Sensor)");
      if (i == 0x3C) Serial.print(" (SSD1306 OLED Display)");
      Serial.println();
      count++;
    }
  }
  if (count == 0) {
    Serial.println("  [WARNING] No I2C devices found! Check wiring: SDA=21, SCL=22, VIN=3.3V, GND.");
  } else {
    Serial.print("  Found "); Serial.print(count); Serial.println(" I2C device(s).\n");
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 05: VL53L0X Laser Time-of-Flight Sensor");
  Serial.println("  Pins: SDA=GPIO 21, SCL=GPIO 22 | Default Addr: 0x29");
  Serial.println("========================================================\n");

  Wire.begin(PIN_TOF_SDA, PIN_TOF_SCL);
  scanI2CBus();

  if (!lox.begin()) {
    Serial.println("[ERROR] Failed to boot VL53L0X sensor. Check 0x29 connection.");
    tofDetected = false;
  } else {
    Serial.println("[TOF] VL53L0X initialized successfully in Continuous Ranging mode!\n");
    tofDetected = true;
    lox.startRangeContinuous();
  }
}

void loop() {
  if (tofDetected) {
    if (lox.isRangeComplete()) {
      uint16_t distanceMm = lox.readRange();
      
      String proximityStatus = "CORRIDOR CLEAR";
      if (distanceMm < 300) {
        proximityStatus = "CRITICAL: HIGH CROWD COMPRESSION AT GATE!";
      } else if (distanceMm < 800) {
        proximityStatus = "PERSON PASSING THROUGH GATE";
      }

      Serial.print("[TOF] Distance: ");
      Serial.print(distanceMm);
      Serial.print(" mm  |  Zone State: ");
      Serial.println(proximityStatus);
    }
  } else {
    Serial.println("[TOF] Sensor offline. Retrying initialization...");
    if (lox.begin()) {
      tofDetected = true;
      lox.startRangeContinuous();
    }
    delay(1500);
  }

  delay(150);
}
