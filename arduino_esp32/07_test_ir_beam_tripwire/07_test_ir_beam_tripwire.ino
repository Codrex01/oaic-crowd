/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 07: Infrared Beam Break Tripwire
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * PIN CONNECTIONS:
 * - IR Transmitter (+): 5V or 3.3V
 * - IR Transmitter (-): GND
 * - IR Receiver (+):    3.3V
 * - IR Receiver (-):    GND
 * - IR Signal Out:      GPIO 14 (Configured with INPUT_PULLUP)
 * 
 * HOW IT WORKS:
 * - When beam is unbroken: Receiver output is HIGH (or LOW depending on active-low/high sensor).
 * - When person breaks beam: Triggers a hardware interrupt on GPIO 14.
 * - Counts cumulative passage events with 200ms debounce protection.
 * =============================================================================
 */

#define PIN_IR_RECEIVER 14

volatile unsigned long totalCrossings = 0;
volatile unsigned long lastTriggerTime = 0;
volatile bool beamBroken = false;

// Hardware Interrupt Service Routine (ISR)
void IRAM_ATTR onBeamBreak() {
  unsigned long now = millis();
  if (now - lastTriggerTime > 250) { // 250ms debounce
    totalCrossings++;
    beamBroken = true;
    lastTriggerTime = now;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 07: IR Beam Break Tripwire");
  Serial.println("  Pin: GPIO 14 (Interrupt with Internal Pull-Up)");
  Serial.println("  Pass your hand through the IR beam to test crossing!");
  Serial.println("========================================================\n");

  pinMode(PIN_IR_RECEIVER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_IR_RECEIVER), onBeamBreak, FALLING);
}

void loop() {
  if (beamBroken) {
    beamBroken = false;
    Serial.print("[TRIPWIRE EVENT] Beam Interrupted! Crossing Count: ");
    Serial.print(totalCrossings);
    Serial.println(" pedestrians passed.");
  }

  // Periodic status
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2500) {
    lastPrint = millis();
    int rawState = digitalRead(PIN_IR_RECEIVER);
    Serial.print("[TRIPWIRE STATUS] Cumulative Crossings: ");
    Serial.print(totalCrossings);
    Serial.print("  |  Current Beam State: ");
    Serial.println(rawState == HIGH ? "BEAM INTACT (CLEAR)" : "BEAM BLOCKED (OBJECT PRESENT)");
  }
}
