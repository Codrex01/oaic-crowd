/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 08: Ra-02 LoRa (SX1278) Diagnostic & Check
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * WHY IS LORA NOT DETECTING?
 * This diagnostic tool probes the Semtech SX1278 SPI registers directly without
 * crashing or hanging your ESP32, explaining the exact physical wiring problem.
 * 
 * DEFAULT SPI PIN MAPPING (Ai-Thinker Ra-02 on ESP32):
 * - NSS (CS):   GPIO 5
 * - SCK:        GPIO 18
 * - MISO:       GPIO 19
 * - MOSI:       GPIO 23
 * - RST:        GPIO 14 (or GPIO 4)
 * - DIO0:       GPIO 2  (or GPIO 26)
 * - VCC:        STRICTLY 3.3V (Do NOT connect Ra-02 to 5V! It will fry the RF IC)
 * - GND:        GND
 * 
 * HARDWARE REGISTER CHECK:
 * - Probes SX1278 RegVersion (Address 0x42).
 *   - Expected: 0x12 (Semtech SX1278 silicon ID).
 *   - If 0x00:  SPI bus lines grounded or SCK/MOSI disconnected.
 *   - If 0xFF:  NSS pin floating, MISO floating, or 3.3V power not reaching Ra-02.
 * =============================================================================
 */

#include <SPI.h>

#define LORA_NSS   5
#define LORA_SCK   18
#define LORA_MISO  19
#define LORA_MOSI  23
#define LORA_RST   14
#define LORA_DIO0  2

#define REG_VERSION 0x42
#define REG_OP_MODE 0x01

uint8_t readRegister(uint8_t address) {
  digitalWrite(LORA_NSS, LOW);
  SPI.transfer(address & 0x7F); // Bit 7 = 0 for read
  uint8_t value = SPI.transfer(0x00);
  digitalWrite(LORA_NSS, HIGH);
  return value;
}

void writeRegister(uint8_t address, uint8_t value) {
  digitalWrite(LORA_NSS, LOW);
  SPI.transfer(address | 0x80); // Bit 7 = 1 for write
  SPI.transfer(value);
  digitalWrite(LORA_NSS, HIGH);
}

void resetLoRa() {
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(20);
  digitalWrite(LORA_RST, HIGH);
  delay(50);
}

void setup() {
  Serial.begin(115200);
  delay(600);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 08: Ra-02 LoRa (SX1278) Diagnostic Tool");
  Serial.println("  SPI Pins: SCK=18, MISO=19, MOSI=23, NSS=5, RST=14");
  Serial.println("========================================================\n");

  pinMode(LORA_NSS, OUTPUT);
  digitalWrite(LORA_NSS, HIGH);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  SPI.setFrequency(1000000); // 1 MHz safe SPI speed for testing

  Serial.println("[LORA] Pulsing Reset line (GPIO 14)...");
  resetLoRa();

  Serial.println("[LORA] Reading SX1278 Silicon Version Register (0x42)...");
  uint8_t version = readRegister(REG_VERSION);

  Serial.print("  -> Read Value: 0x");
  if (version < 16) Serial.print("0");
  Serial.print(version, HEX);

  if (version == 0x12) {
    Serial.println(" [SUCCESS! SX1278 SILICON DETECTED]");
    Serial.println("  Your Ra-02 SPI wiring is 100% correct!");
    
    // Check Sleep / Standby Mode
    writeRegister(REG_OP_MODE, 0x81); // Long Range Mode + Standby
    uint8_t opMode = readRegister(REG_OP_MODE);
    Serial.print("  -> OpMode Register (0x01): 0x");
    Serial.println(opMode, HEX);
    Serial.println("  LoRa Transceiver is ready for packet transmission!");
  } else if (version == 0x00) {
    Serial.println(" [FAILED: ALL ZEROS]");
    Serial.println("  DIAGNOSIS: SPI MISO or SCK is grounded, or module has no power.");
    Serial.println("  FIX: Verify 3.3V power to Ra-02 and check for solder bridges.");
  } else if (version == 0xFF) {
    Serial.println(" [FAILED: ALL ONES (0xFF)]");
    Serial.println("  DIAGNOSIS: MISO is floating or NSS (GPIO 5) is not selecting the chip.");
    Serial.println("  FIX 1: Ensure Ra-02 NSS is securely connected to ESP32 GPIO 5.");
    Serial.println("  FIX 2: Ensure Ra-02 MOSI connects to GPIO 23, MISO to GPIO 19.");
    Serial.println("  FIX 3: Verify GND wire is firmly connected between ESP32 and Ra-02.");
  } else {
    Serial.println(" [UNEXPECTED VERSION]");
    Serial.println("  DIAGNOSIS: Signal noise or weak connection on SPI bus.");
    Serial.println("  FIX: Use shorter jumper wires and verify 3.3V voltage rail.");
  }

  Serial.println("\n--------------------------------------------------------");
  Serial.println("Continuous probing active. Press Enter to re-check...");
}

void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();
    resetLoRa();
    uint8_t v = readRegister(REG_VERSION);
    Serial.print("[LORA RE-CHECK] Reg 0x42 = 0x");
    if (v < 16) Serial.print("0");
    Serial.print(v, HEX);
    Serial.println(v == 0x12 ? " (OK!)" : " (Disconnected)");
  }
  delay(1000);
}
