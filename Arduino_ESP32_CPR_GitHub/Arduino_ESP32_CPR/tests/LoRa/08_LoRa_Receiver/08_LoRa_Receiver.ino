// Upload to the second ESP32 only after SPI diagnostic succeeds.
// Receiver and transmitter must use matching frequency/settings.
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

void setup() {
  Serial.begin(115200);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  Serial.println("Starting LoRa receiver...");
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("LoRa init failed. Stop and re-check SPI/power/pins.");
    while (true) delay(1000);
  }
  Serial.println("LoRa receiver ready at 433 MHz.");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received: ");
    while (LoRa.available()) Serial.print((char)LoRa.read());
    Serial.printf("\nRSSI: %d dBm | SNR: %.1f dB\n",
                  LoRa.packetRssi(), LoRa.packetSnr());
  }
}
