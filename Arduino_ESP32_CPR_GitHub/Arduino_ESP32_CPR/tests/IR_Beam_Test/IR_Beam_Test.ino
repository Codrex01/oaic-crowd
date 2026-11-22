/*
  IR beam sensor test TEMPLATE.
  IMPORTANT: Your IR sensor relay/output voltage and wiring are not confirmed.
  Do NOT connect a 12V/24V output directly to an ESP32 GPIO.
  Use a properly rated optocoupler/level interface or relay contact wiring.
  GPIO34 is input-only and has no internal pull-up on classic ESP32.
  Configure the interface so the ESP32 pin sees ONLY 0V or 3.3V.
*/

const int IR_INPUT_PIN = 34; // change after confirming safe interface
const int ACTIVE_LEVEL = LOW; // change depending on NO/NC/contact wiring

int previousState = HIGH;
unsigned long eventCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(IR_INPUT_PIN, INPUT); // external pull-up/down may be required
  previousState = digitalRead(IR_INPUT_PIN);
  Serial.println("IR beam test template started.");
  Serial.println("Only continue after confirming GPIO voltage is 0-3.3V.");
}

void loop() {
  int currentState = digitalRead(IR_INPUT_PIN);

  if (currentState != previousState) {
    delay(30); // basic debounce
    currentState = digitalRead(IR_INPUT_PIN);

    if (currentState != previousState) {
      previousState = currentState;
      if (currentState == ACTIVE_LEVEL) {
        eventCount++;
        Serial.printf("Beam event detected. Event count: %lu\n", eventCount);
      } else {
        Serial.println("Beam restored.");
      }
    }
  }
  delay(5);
}
