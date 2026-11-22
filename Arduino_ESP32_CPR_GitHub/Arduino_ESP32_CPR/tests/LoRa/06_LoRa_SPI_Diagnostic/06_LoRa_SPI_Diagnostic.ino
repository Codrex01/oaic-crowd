// RA-02 / SX1278 SPI register diagnostic.
// RA-02 VCC MUST be 3.3V, never 5V.
// Expected SX1278 version register (0x42): commonly 0x12.

#include <SPI.h>

static const int LORA_SCK  = 18;
static const int LORA_MISO = 19;
static const int LORA_MOSI = 23;
static const int LORA_NSS  = 13;
static const int LORA_RST  = 27;

SPISettings radioSPI(1000000, MSBFIRST, SPI_MODE0);

uint8_t readRegister(uint8_t address) {
  SPI.beginTransaction(radioSPI);
  digitalWrite(LORA_NSS, LOW);
  SPI.transfer(address & 0x7F);
  uint8_t value = SPI.transfer(0x00);
  digitalWrite(LORA_NSS, HIGH);
  SPI.endTransaction();
  return value;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LORA_NSS, OUTPUT);
  digitalWrite(LORA_NSS, HIGH);
  pinMode(LORA_RST, OUTPUT);

  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(10);

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

  Serial.println("CPR - SX1278 REGISTER TEST");
  uint8_t version = readRegister(0x42);
  Serial.printf("SX1278 version register: 0x%02X\n", version);

  if (version == 0x12) {
    Serial.println("SUCCESS: SX127x responded.");
  } else if (version == 0xFF) {
    Serial.println("ERROR: 0xFF - likely no SPI response / MISO high.");
  } else if (version == 0x00) {
    Serial.println("ERROR: 0x00 - check power, reset, NSS and wiring.");
  } else {
    Serial.println("Unexpected version. Verify module and wiring.");
  }
}

void loop() {}
