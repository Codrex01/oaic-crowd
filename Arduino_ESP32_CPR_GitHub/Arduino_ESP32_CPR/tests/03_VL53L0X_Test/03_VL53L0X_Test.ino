#include <Wire.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X sensor = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL
  Serial.println("VL53L0X test");

  if (!sensor.begin(0x29)) {
    Serial.println("ERROR: VL53L0X not found at 0x29.");
    while (true) delay(100);
  }
  Serial.println("VL53L0X detected.");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  sensor.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) {
    Serial.printf("Distance: %u mm (%.2f cm)\n",
                  measure.RangeMilliMeter,
                  measure.RangeMilliMeter / 10.0);
  } else {
    Serial.println("Out of range / invalid measurement.");
  }
  delay(300);
}
