/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 04: 0.96" SSD1306 I2C OLED Screen
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * PIN CONNECTIONS:
 * - OLED VCC: 3.3V (or 5V if board has onboard 3.3V LDO)
 * - OLED GND: GND
 * - OLED SDA: GPIO 21
 * - OLED SCL: GPIO 22
 * - I2C Address: 0x3C (Standard)
 * 
 * REQUIRED LIBRARIES:
 * - "Adafruit SSD1306" by Adafruit
 * - "Adafruit GFX Library" by Adafruit
 * 
 * DEMONSTRATION MODES:
 * - Automatically cycles through the 3 CPR Exhibition display screens:
 *   1. GREEN / NORMAL:   "CPR RADAR: LIVE", "COUNT: 2 PPL", "GATE: OPEN (90D)"
 *   2. YELLOW / WARNING: "CPR: WARNING",    "COUNT: 7 PPL", "GATE: METER (45D)"
 *   3. RED / EMERGENCY:  "CPR: RED ALERT!", "COUNT: 14 PPL","GATE: LOCKED (0D)"
 * =============================================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define PIN_OLED_SDA 21
#define PIN_OLED_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int currentMode = 0;
unsigned long lastSwitch = 0;

void drawCprScreen(const char* header, const char* countStr, const char* statusStr, const char* gateStr, bool invertedHeader) {
  display.clearDisplay();

  // 1. Top Inverted Header Banner
  if (invertedHeader) {
    display.fillRect(0, 0, 128, 14, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.drawRect(0, 0, 128, 14, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);
  }
  display.setTextSize(1);
  display.setCursor(4, 3);
  display.println(header);

  // 2. Center Large Crowd Count
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.println(countStr);

  // 3. Status Level
  display.setTextSize(1);
  display.setCursor(0, 38);
  display.println(statusStr);

  // 4. Physical Gate Position
  display.setCursor(0, 50);
  display.println(gateStr);

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 04: 0.96 inch SSD1306 OLED (128x64)");
  Serial.println("  I2C Pins: SDA=GPIO 21, SCL=GPIO 22 | Addr: 0x3C");
  Serial.println("========================================================\n");

  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("[ERROR] SSD1306 allocation failed. Check I2C wiring (SDA=21, SCL=22) and address (0x3C).");
    while (true) delay(100);
  }

  Serial.println("[OLED] Display initialized successfully!");

  // Boot Splash
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(8, 8);
  display.println("CROWD PRESSURE RADAR");
  display.setCursor(8, 24);
  display.println("EXHIBITION v3.0");
  display.setCursor(8, 40);
  display.println("OLED DISPLAY: OK");
  display.display();
  delay(2000);
}

void loop() {
  if (millis() - lastSwitch > 3000) {
    lastSwitch = millis();
    currentMode = (currentMode + 1) % 3;

    if (currentMode == 0) {
      Serial.println("[OLED] Screen 1: GREEN (Normal Flow)");
      drawCprScreen("CPR RADAR: LIVE", "COUNT: 2 PPL", "STATUS: NORMAL", "GATE: OPEN (90D)", true);
    } else if (currentMode == 1) {
      Serial.println("[OLED] Screen 2: YELLOW (Caution Inflow Metering)");
      drawCprScreen("CPR: WARNING", "COUNT: 7 PPL", "STATUS: ELEVATED", "GATE: METER (45D)", true);
    } else if (currentMode == 2) {
      Serial.println("[OLED] Screen 3: RED (Emergency Lockdown)");
      drawCprScreen("CPR: RED ALERT!", "COUNT: 14 PPL", "STATUS: CRITICAL", "GATE: LOCKED (0D)", true);
    }
  }
}
