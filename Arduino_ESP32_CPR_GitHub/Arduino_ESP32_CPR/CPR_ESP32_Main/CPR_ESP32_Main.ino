/*
  CPR_ESP32_Main
  Combined ESP32 firmware for currently usable prototype components.

  Included:
  - HLK-LD2410C OUT presence signal + UART raw monitor
  - VL53L0X distance sensor
  - SSD1306 OLED
  - status LED + active buzzer
  - SG90 servo (manual serial commands only)
  - optional Wi-Fi connection status

  IMPORTANT:
  - This does not count people or measure physical crowd pressure.
  - YOLO person detection/counting runs separately on the computer.
  - Change LED_PIN / BUZZER_PIN to match your actual wiring.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_VL53L0X.h>
#include <ESP32Servo.h>
#include <WiFi.h>

// ---------------- USER CONFIGURATION ----------------
#define ENABLE_WIFI false
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Pins based on the current CPR wiring plan.
// Verify against your physical ESP32 board before upload.
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;

static const int HLK_UART_RX_PIN = 16;  // ESP32 RX <- HLK TX
static const int HLK_UART_TX_PIN = 17;  // ESP32 TX -> HLK RX
static const int HLK_OUT_PIN = 32;      // HLK OUT -> ESP32 input

static const int SERVO_PIN = 25;
static const int LED_PIN = 4;           // CHANGE if your LED is wired elsewhere
static const int BUZZER_PIN = 33;       // CHANGE if your buzzer is wired elsewhere

static const uint32_t HLK_BAUD = 256000;
static const uint8_t OLED_ADDRESS = 0x3C;
static const int OLED_WIDTH = 128;
static const int OLED_HEIGHT = 64;

// VL53L0X proximity thresholds. These are object-distance demo thresholds,
// NOT crowd-density or crowd-pressure thresholds.
static const uint16_t NEAR_DISTANCE_MM = 500;
static const uint16_t CAUTION_DISTANCE_MM = 1200;

// Servo demo positions. Adjust to your mechanical linkage.
static const int SERVO_OPEN_ANGLE = 90;
static const int SERVO_CLOSED_ANGLE = 0;
// ----------------------------------------------------

HardwareSerial RadarSerial(2);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
Adafruit_VL53L0X tof = Adafruit_VL53L0X();
Servo gateServo;

bool oledReady = false;
bool tofReady = false;
bool servoReady = false;
bool radarPresent = false;
bool nearObject = false;
bool cautionObject = false;
uint16_t distanceMM = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastSerialUpdate = 0;

void setStatusOutputs() {
  // A presence indication alone is not treated as an alarm.
  // The distance thresholds only indicate a nearby object.
  if (nearObject) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH); // suitable only for a GPIO-compatible active buzzer/driver
  } else if (cautionObject) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, radarPresent ? HIGH : LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void updateOLED() {
  if (!oledReady) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("CPR - ESP32 MONITOR");
  display.println("---------------------");

  display.print("Radar presence: ");
  display.println(radarPresent ? "YES" : "NO");

  display.print("ToF: ");
  if (tofReady && distanceMM > 0) {
    display.print(distanceMM);
    display.println(" mm");
  } else {
    display.println("Unavailable");
  }

  display.print("Object range: ");
  if (nearObject) display.println("NEAR");
  else if (cautionObject) display.println("CAUTION");
  else display.println("CLEAR");

  display.println();
  display.println("Servo: serial O/C");
  display.display();
}

void readSensors() {
  radarPresent = (digitalRead(HLK_OUT_PIN) == HIGH);

  if (tofReady) {
    VL53L0X_RangingMeasurementData_t measure;
    tof.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {
      distanceMM = measure.RangeMilliMeter;
      nearObject = (distanceMM > 0 && distanceMM < NEAR_DISTANCE_MM);
      cautionObject = (distanceMM >= NEAR_DISTANCE_MM &&
                       distanceMM < CAUTION_DISTANCE_MM);
    } else {
      distanceMM = 0;
      nearObject = false;
      cautionObject = false;
    }
  }

  setStatusOutputs();
}

void printSnapshot() {
  Serial.println();
  Serial.println("========== CPR SENSOR SNAPSHOT ==========");
  Serial.printf("HLK OUT presence: %s\n", radarPresent ? "PRESENT" : "NOT PRESENT");

  if (tofReady && distanceMM > 0) {
    Serial.printf("VL53L0X distance: %u mm\n", distanceMM);
  } else {
    Serial.println("VL53L0X distance: unavailable / out of range");
  }

  Serial.printf("Near-object flag: %s\n", nearObject ? "YES" : "NO");
  Serial.printf("Caution-object flag: %s\n", cautionObject ? "YES" : "NO");
  Serial.println("NOTE: These readings do not represent crowd pressure.");
  Serial.println("=========================================");
}

void handleSerialCommands() {
  while (Serial.available()) {
    char command = Serial.read();

    if (command == 'O' || command == 'o') {
      if (servoReady) {
        gateServo.write(SERVO_OPEN_ANGLE);
        Serial.println("Demo servo -> OPEN position");
      } else {
        Serial.println("Servo not initialized.");
      }
    } else if (command == 'C' || command == 'c') {
      if (servoReady) {
        gateServo.write(SERVO_CLOSED_ANGLE);
        Serial.println("Demo servo -> CLOSED position");
      } else {
        Serial.println("Servo not initialized.");
      }
    } else if (command == 'B' || command == 'b') {
      Serial.println("Buzzer test pulse");
      digitalWrite(BUZZER_PIN, HIGH);
      delay(150);
      digitalWrite(BUZZER_PIN, LOW);
    } else if (command == 'S' || command == 's') {
      printSnapshot();
    } else if (command == '\n' || command == '\r') {
      // ignore line endings
    } else {
      Serial.println("Commands: O=open, C=close, B=buzzer test, S=snapshot");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("==================================");
  Serial.println(" CPR ESP32 MAIN FIRMWARE");
  Serial.println("==================================");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(HLK_OUT_PIN, INPUT);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (oledReady) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("CPR booting...");
    display.display();
    Serial.println("[OK] SSD1306 OLED found at 0x3C");
  } else {
    Serial.println("[WARN] OLED not found. Check controller/address/wiring.");
  }

  tofReady = tof.begin(0x29, false, &Wire);
  Serial.println(tofReady ? "[OK] VL53L0X found at 0x29"
                          : "[WARN] VL53L0X not found.");

  RadarSerial.begin(HLK_BAUD, SERIAL_8N1, HLK_UART_RX_PIN, HLK_UART_TX_PIN);
  Serial.println("[INFO] HLK UART opened at 256000 baud.");
  Serial.println("[INFO] HLK OUT pin is used for simple presence detection.");

  gateServo.setPeriodHertz(50);
  gateServo.attach(SERVO_PIN, 500, 2400);
  servoReady = gateServo.attached();
  if (servoReady) {
    gateServo.write(SERVO_CLOSED_ANGLE);
    Serial.println("[OK] Servo attached. Use O/C in Serial Monitor.");
  } else {
    Serial.println("[WARN] Servo attach failed. Check GPIO/power.");
  }

  if (ENABLE_WIFI) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[INFO] Connecting to Wi-Fi");
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      delay(300);
      Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("[OK] Wi-Fi connected. IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("[WARN] Wi-Fi connection failed.");
    }
  }

  Serial.println();
  Serial.println("Commands: O=open, C=close, B=buzzer test, S=snapshot");
  Serial.println("Do not treat proximity readings as crowd pressure.");
}

void loop() {
  readSensors();

  // Raw UART bytes are displayed for diagnostics. This is not a decoded
  // LD2410C protocol parser.
  while (RadarSerial.available()) {
    uint8_t value = RadarSerial.read();
    Serial.printf("[HLK UART] 0x%02X\n", value);
  }

  handleSerialCommands();

  if (millis() - lastSerialUpdate >= 1000) {
    lastSerialUpdate = millis();
    Serial.printf("Presence=%s | ToF=%s",
                  radarPresent ? "YES" : "NO",
                  (tofReady && distanceMM > 0) ? String(distanceMM).c_str() : "N/A");
    Serial.println(" mm");
  }

  if (millis() - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = millis();
    updateOLED();
  }

  delay(50);
}
