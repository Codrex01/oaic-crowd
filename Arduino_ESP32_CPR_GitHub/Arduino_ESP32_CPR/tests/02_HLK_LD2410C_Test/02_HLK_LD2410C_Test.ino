// HLK-LD2410C simple presence + UART monitor.
// Sensor TX -> ESP32 GPIO16 (RX)
// Sensor RX -> ESP32 GPIO17 (TX)
// Sensor OUT -> ESP32 GPIO32
// Sensor VCC -> regulated 5V supply; GND -> common GND.
// UART default: 256000 baud, 8N1.

HardwareSerial RadarSerial(2);
const int RADAR_RX = 16;
const int RADAR_TX = 17;
const int RADAR_OUT = 32;

void setup() {
  Serial.begin(115200);
  pinMode(RADAR_OUT, INPUT);
  RadarSerial.begin(256000, SERIAL_8N1, RADAR_RX, RADAR_TX);
  Serial.println("HLK-LD2410C test started.");
  Serial.println("OUT pin gives a simple presence indication.");
  Serial.println("UART output below is raw bytes, not decoded measurements.");
}

void loop() {
  Serial.printf("Presence OUT: %s\n",
                digitalRead(RADAR_OUT) ? "HIGH - presence" : "LOW - no presence");

  while (RadarSerial.available()) {
    uint8_t b = RadarSerial.read();
    Serial.printf("UART byte: 0x%02X (%u)\n", b, b);
  }
  delay(500);
}
