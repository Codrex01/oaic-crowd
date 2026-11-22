/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 03: SG90 Micro Servo Gate
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * PIN CONNECTIONS:
 * - Servo Signal (Orange / Yellow): GPIO 13
 * - Servo VCC (Red):                 5V (or Vin from USB)
 * - Servo GND (Brown / Black):       GND
 * 
 * REQUIRED LIBRARY:
 * - "ESP32Servo" by Kevin Harrington (Install via Arduino Library Manager)
 * 
 * SERIAL MONITOR COMMANDS (115200 baud):
 * - Send '9' -> 90 degrees (GATE OPEN — Normal flow)
 * - Send '4' -> 45 degrees (GATE METERED — Regulated caution inflow)
 * - Send '0' -> 0 degrees  (GATE LOCKED — Emergency crowd crush shield)
 * - Send 's' -> SWEEP MODE (Automated continuous test sweep 0 to 90 deg)
 * =============================================================================
 */

#include <ESP32Servo.h>

#define PIN_SERVO 13

Servo gateServo;

int currentAngle = 90;
bool sweepMode = false;
int sweepStep = 1;

void setGateAngle(int angle) {
  currentAngle = constrain(angle, 0, 90);
  gateServo.write(currentAngle);
  
  String desc = "UNKNOWN";
  if (currentAngle == 90) desc = "OPEN (Normal Pedestrian Flow | < 5 People)";
  else if (currentAngle == 45) desc = "METERED (Regulated Inflow | 5 - 9 People)";
  else if (currentAngle == 0) desc = "CLOSED & LOCKED (Crush Shield Active | >= 10 People)";
  
  Serial.print("[SERVO GATE] Position: ");
  Serial.print(currentAngle);
  Serial.print(" deg -> ");
  Serial.println(desc);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 03: SG90 Servo Corridor Barrier");
  Serial.println("  Pin: GPIO 13 (PWM)");
  Serial.println("  Commands: '9' (Open), '4' (Meter), '0' (Lock), 's' (Sweep)");
  Serial.println("========================================================\n");

  gateServo.setPeriodHertz(50);             // Standard 50Hz servo PWM
  gateServo.attach(PIN_SERVO, 500, 2400);   // Standard 500-2400us pulse bounds

  setGateAngle(90); // Default Open
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '9') { sweepMode = false; setGateAngle(90); }
    else if (cmd == '4') { sweepMode = false; setGateAngle(45); }
    else if (cmd == '0') { sweepMode = false; setGateAngle(0); }
    else if (cmd == 's' || cmd == 'S') {
      sweepMode = !sweepMode;
      Serial.print("[SERVO] Sweep mode: ");
      Serial.println(sweepMode ? "ENABLED" : "DISABLED");
    }
  }

  // Smooth sweep between 0 and 90 degrees
  if (sweepMode) {
    currentAngle += sweepStep;
    if (currentAngle >= 90) { currentAngle = 90; sweepStep = -1; delay(400); }
    if (currentAngle <= 0)  { currentAngle = 0;  sweepStep = 1;  delay(400); }
    gateServo.write(currentAngle);
    delay(20);
  }
}
