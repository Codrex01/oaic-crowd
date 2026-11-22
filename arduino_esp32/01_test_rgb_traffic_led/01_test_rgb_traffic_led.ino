/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 01: 3-in-1 RGB & Traffic LEDs
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * PIN CONNECTIONS:
 * - Red Channel:   GPIO 25  (PWM Channel 0)
 * - Green Channel: GPIO 26  (PWM Channel 1)
 * - Blue Channel:  GPIO 27  (PWM Channel 2)
 * - Common Pin:    GND (if Common Cathode) or 3.3V (if Common Anode)
 * 
 * SERIAL MONITOR COMMANDS (115200 baud):
 * - Send 'g' -> GREEN  (Normal flow, < 5 people)
 * - Send 'y' -> YELLOW (Caution, 5 - 9 people)
 * - Send 'r' -> RED    (Critical crush alert, >= 10 people)
 * - Send 'b' -> BLUE   (Standby / System Boot)
 * - Send 'a' -> AUTO   (Cycles all safety states automatically)
 * =============================================================================
 */

#define PIN_RED    25
#define PIN_GREEN  26
#define PIN_BLUE   27

// Set to true if your RGB LED is Common Anode (+3.3V/5V common pin)
// Set to false if your RGB LED is Common Cathode (GND common pin - most common)
#define RGB_COMMON_ANODE false

// ESP32 PWM LEDC Settings
const int PWM_CH_RED   = 0;
const int PWM_CH_GREEN = 1;
const int PWM_CH_BLUE  = 2;
const int PWM_FREQ     = 5000;
const int PWM_RES      = 8; // 8-bit: 0 to 255

bool autoCycle = true;
unsigned long lastCycleTime = 0;
int cycleStep = 0;

void setRgbColor(uint8_t r, uint8_t g, uint8_t b) {
  if (RGB_COMMON_ANODE) {
    ledcWrite(PWM_CH_RED, 255 - r);
    ledcWrite(PWM_CH_GREEN, 255 - g);
    ledcWrite(PWM_CH_BLUE, 255 - b);
  } else {
    ledcWrite(PWM_CH_RED, r);
    ledcWrite(PWM_CH_GREEN, g);
    ledcWrite(PWM_CH_BLUE, b);
  }
}

void showGreen() {
  Serial.println("[LED] State: GREEN (< 5 People | Safe Flow | Gate Open 90 deg)");
  setRgbColor(0, 255, 0);
}

void showYellow() {
  Serial.println("[LED] State: YELLOW/AMBER (5 - 9 People | Caution Metering | Gate 45 deg)");
  // Rich optical amber mix: full red + warm green
  setRgbColor(255, 180, 0);
}

void showRed() {
  Serial.println("[LED] State: RED (>= 10 People | CRITICAL CRUSH ALERT | Gate Locked 0 deg)");
  setRgbColor(255, 0, 0);
}

void showBlue() {
  Serial.println("[LED] State: BLUE (System Booting / Standby)");
  setRgbColor(0, 80, 255);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 01: 3-in-1 RGB Traffic Light");
  Serial.println("  Pins: RED=25, GREEN=26, BLUE=27");
  Serial.println("  Type 'g', 'y', 'r', 'b', or 'a' in Serial Monitor");
  Serial.println("========================================================\n");

  ledcSetup(PWM_CH_RED, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_GREEN, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_BLUE, PWM_FREQ, PWM_RES);

  ledcAttachPin(PIN_RED, PWM_CH_RED);
  ledcAttachPin(PIN_GREEN, PWM_CH_GREEN);
  ledcAttachPin(PIN_BLUE, PWM_CH_BLUE);

  showBlue();
  delay(1000);
}

void loop() {
  // Check Serial Monitor for manual color commands
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'g' || cmd == 'G') { autoCycle = false; showGreen(); }
    else if (cmd == 'y' || cmd == 'Y') { autoCycle = false; showYellow(); }
    else if (cmd == 'r' || cmd == 'R') { autoCycle = false; showRed(); }
    else if (cmd == 'b' || cmd == 'B') { autoCycle = false; showBlue(); }
    else if (cmd == 'a' || cmd == 'A') {
      autoCycle = true;
      Serial.println("[LED] Resuming automatic safety cycle...");
    }
  }

  // Automatic state demo cycle
  if (autoCycle && millis() - lastCycleTime > 2500) {
    lastCycleTime = millis();
    cycleStep = (cycleStep + 1) % 3;

    if (cycleStep == 0) showGreen();
    else if (cycleStep == 1) showYellow();
    else if (cycleStep == 2) showRed();
  }
}
