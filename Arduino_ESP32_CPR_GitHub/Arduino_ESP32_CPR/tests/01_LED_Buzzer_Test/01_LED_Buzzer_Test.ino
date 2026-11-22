// Standalone LED + active buzzer test.
// Change these pins to match your actual wiring.
const int LED_PIN = 4;
const int BUZZER_PIN = 33;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("LED + active buzzer test started.");
}

void loop() {
  Serial.println("LED ON, buzzer ON");
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);

  Serial.println("LED OFF, buzzer OFF");
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
}
