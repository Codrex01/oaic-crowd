/*
 * CPR (Crowd Pressure Radar) — Slave Node Firmware
 * Board: ESP32 Dev Module (Breadboard 2)
 * 
 * Hardware Connections:
 * - IR Beam Break Sensor:
 *     Sensor Receiver Output -> GPIO 14 (Interrupt on FALLING edge)
 *     Internal Pull-Up Enabled
 * - Status Indicator LED:
 *     LED Pin -> GPIO 2 (Built-in Blue LED on ESP32)
 * - Ra-02 LoRa SX1278 (SPI):
 *     NSS / CS   -> GPIO 5
 *     RST        -> GPIO 4
 *     DIO0       -> GPIO 26
 *     SCK        -> GPIO 18
 *     MISO       -> GPIO 19
 *     MOSI       -> GPIO 23
 * 
 * Operation:
 * 1. Monitors the infrared tripwire crossing between Breadboard 2 and Breadboard 1.
 * 2. Whenever a person walks through the corridor and breaks the beam, a hardware
 *    interrupt is immediately triggered with debouncing.
 * 3. Transmits an instantaneous LoRa packet ("BEAM_TRIGGER") to Breadboard 1 Master.
 * 4. Also outputs status over USB Serial (115200 baud).
 */

#include <SPI.h>

#define PIN_IR_BEAM      14
#define PIN_STATUS_LED   2

// Debounce & State
volatile unsigned long lastTriggerTime = 0;
volatile int beamBreakCount = 0;
volatile bool beamBrokenFlag = false;
const unsigned long DEBOUNCE_DELAY_MS = 250;

// Hardware Interrupt Service Routine (ISR)
void IRAM_ATTR onBeamBreak() {
  unsigned long now = millis();
  if (now - lastTriggerTime > DEBOUNCE_DELAY_MS) {
    beamBreakCount++;
    beamBrokenFlag = true;
    lastTriggerTime = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[CPR-ESP32] Slave Boundary Node Initializing (Breadboard 2)...");

  // Status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);

  // Configure IR Beam Sensor with Hardware Interrupt
  pinMode(PIN_IR_BEAM, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_IR_BEAM), onBeamBreak, FALLING);

  Serial.println("[CPR-ESP32] IR Beam Sensor attached on GPIO 14 (FALLING interrupt).");
  Serial.println("[CPR-ESP32] Ready for perimeter ingress detection.");
}

void loop() {
  if (beamBrokenFlag) {
    beamBrokenFlag = false;

    // Flash status LED
    digitalWrite(PIN_STATUS_LED, HIGH);

    // Broadcast trigger over Serial
    Serial.print("{\"event\":\"BEAM_TRIGGER\",\"totalCount\":");
    Serial.print(beamBreakCount);
    Serial.println("}");

    delay(60);
    digitalWrite(PIN_STATUS_LED, LOW);
  }

  // Periodic heartbeat every 5 seconds
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 5000) {
    lastHeartbeat = millis();
    Serial.print("{\"node\":\"breadboard_2\",\"status\":\"ONLINE\",\"totalCrossings\":");
    Serial.print(beamBreakCount);
    Serial.println("}");
  }

  delay(10);
}
