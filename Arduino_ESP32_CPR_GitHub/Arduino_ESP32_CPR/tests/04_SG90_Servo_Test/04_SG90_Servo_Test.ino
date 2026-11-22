#include <ESP32Servo.h>

Servo testServo;
const int SERVO_PIN = 25;

void setup() {
  Serial.begin(115200);
  testServo.setPeriodHertz(50);
  testServo.attach(SERVO_PIN, 500, 2400);

  if (!testServo.attached()) {
    Serial.println("Servo attach failed. Check signal pin.");
    while (true) delay(100);
  }

  Serial.println("SG90 test. Servo will move 0 -> 90 -> 180 -> 90.");
  Serial.println("Use external regulated 5V and common GND.");
}

void loop() {
  testServo.write(0);
  delay(1000);
  testServo.write(90);
  delay(1000);
  testServo.write(180);
  delay(1000);
  testServo.write(90);
  delay(1000);
}
