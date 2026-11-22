/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 02: Active Buzzer Alarm
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * PIN CONNECTIONS:
 * - Buzzer (+) Pin: GPIO 12 (Active Buzzer or Transistor Driver)
 * - Buzzer (-) Pin: GND
 * 
 * SERIAL MONITOR COMMANDS (115200 baud):
 * - Send '1' -> Short beep (System armed / event tick)
 * - Send '2' -> Double warning beep (Yellow status / Metered inflow)
 * - Send '3' -> Emergency Pulsed Alarm (Red alert / Crowd crush danger)
 * - Send '0' -> Silence Buzzer
 * =============================================================================
 */

#define PIN_BUZZER 12

int alarmMode = 1; // Default: Single test beeps
unsigned long lastBeepTime = 0;
bool buzzerState = false;

void beep(int durationMs) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  digitalWrite(PIN_BUZZER, LOW);
}

void doubleBeep() {
  beep(80);
  delay(80);
  beep(80);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 02: Active Buzzer Alarm");
  Serial.println("  Pin: GPIO 12");
  Serial.println("  Type '1' (single), '2' (double), '3' (emergency), '0' (off)");
  Serial.println("========================================================\n");

  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);

  // Initial startup chirp
  Serial.println("[BUZZER] Startup chirp...");
  beep(100);
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '1') {
      alarmMode = 1;
      Serial.println("[BUZZER] Mode 1: Confirmation Beep");
      beep(100);
    } else if (cmd == '2') {
      alarmMode = 2;
      Serial.println("[BUZZER] Mode 2: Caution Double Beep");
      doubleBeep();
    } else if (cmd == '3') {
      alarmMode = 3;
      Serial.println("[BUZZER] Mode 3: Continuous Emergency Pulsed Alarm (Active!)");
    } else if (cmd == '0') {
      alarmMode = 0;
      digitalWrite(PIN_BUZZER, LOW);
      Serial.println("[BUZZER] Mode 0: Silenced.");
    }
  }

  // Emergency pulsing pattern
  if (alarmMode == 3) {
    if (millis() - lastBeepTime > 150) {
      lastBeepTime = millis();
      buzzerState = !buzzerState;
      digitalWrite(PIN_BUZZER, buzzerState ? HIGH : LOW);
    }
  }
}
