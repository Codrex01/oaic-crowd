// Use only AFTER the SPI diagnostic returns a valid SX127x version.
// Attach the correct 433 MHz antenna BEFORE powering/transmitting.
// RA-02 VCC = 3.3V ONLY.

#include <SPI.h>
#include <LoRa.h>

const long LORA_FREQUENCY = 433E6;
const int LORA_SCK = 18;
const int LORA_MISO = 19;
const int LORA_MOSI = 23;
const int LORA_SS = 13;
const int LORA_RST = 27;
const int LORA_DIO0 = 26;

unsigned long counter = 0;

void setup() {
  Serial.begin(115200);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  Serial.println("Starting LoRa transmitter...");
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("LoRa init failed. Stop and re-check SPI/power/pins.");
    while (true) delay(1000);
  }
  Serial.println("LoRa transmitter ready at 433 MHz.");
}

void loop() {
  LoRa.beginPacket();
  LoRa.print("CPR_TEST_PACKET_");
  LoRa.print(counter++);
  int result = LoRa.endPacket();

  Serial.printf("Packet transmit result: %s\n", result ? "OK" : "FAILED");
  delay(2000);
}
