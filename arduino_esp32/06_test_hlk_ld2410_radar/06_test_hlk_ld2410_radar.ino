/*
 * =============================================================================
 * CPR (Crowd Pressure Radar) — Hardware Test 06: HLK-LD2410 24GHz mmWave Radar
 * Board: ESP32 Dev Module
 * =============================================================================
 * 
 * PIN CONNECTIONS:
 * - Radar VCC: 5V (Recommended for stable 24GHz RF synthesis, or strong 3.3V)
 * - Radar GND: GND
 * - Radar TX:  GPIO 16 (ESP32 RX2)
 * - Radar RX:  GPIO 17 (ESP32 TX2)
 * 
 * PROTOCOL & BAUD:
 * - Baud Rate: 256000 baud (Default factory rate for HLK-LD2410 / LD2410B / LD2410C)
 * - Uses ESP32 HardwareSerial(2)
 * 
 * FRAME PARSING:
 * - Frame Header: 0xF4 0xF3 0xF2 0xF1
 * - Frame Tail:   0xF8 0xF7 0xF6 0xF5
 * - Extracts:
 *   1. Target State: (0=None, 1=Moving, 2=Stationary, 3=Moving & Stationary)
 *   2. Moving Target Distance (cm) & Energy (0 - 100%)
 *   3. Stationary Target Distance (cm) & Energy (0 - 100%)
 * =============================================================================
 */

#define RADAR_RX_PIN 16
#define RADAR_TX_PIN 17

HardwareSerial RadarSerial(2);

// LD2410 Frame Header
const uint8_t FRAME_HEADER[4] = {0xF4, 0xF3, 0xF2, 0xF1};
const uint8_t FRAME_TAIL[4]   = {0xF8, 0xF7, 0xF6, 0xF5};

uint8_t buffer[64];
int bufIndex = 0;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========================================================");
  Serial.println("  CPR ESP32 Test 06: HLK-LD2410 24GHz FMCW Radar");
  Serial.println("  Pins: Radar TX -> GPIO 16 (RX2), Radar RX -> GPIO 17 (TX2)");
  Serial.println("  Baud: 256000 baud");
  Serial.println("========================================================\n");

  RadarSerial.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  Serial.println("[RADAR] Listening for 24GHz millimeter-wave chirp frames...\n");
}

void parseRadarFrame(uint8_t* frame, int len) {
  // Ensure basic payload length (at least 23 bytes)
  if (len < 18) return;

  // Frame structure:
  // [0..3]: Header (F4 F3 F2 F1)
  // [4..5]: Length
  // [6]: DataType (0x02 = Target Data)
  // [7]: Target State (0=None, 1=Moving, 2=Stationary, 3=Both)
  // [8..9]: Moving Distance (cm, uint16_le)
  // [10]: Moving Energy (0-100)
  // [11..12]: Stationary Distance (cm, uint16_le)
  // [13]: Stationary Energy (0-100)
  // [14..15]: Detection Distance (cm)

  uint8_t targetState = frame[7];
  uint16_t movingDist = frame[8] | (frame[9] << 8);
  uint8_t movingEnergy = frame[10];
  uint16_t statDist = frame[11] | (frame[12] << 8);
  uint8_t statEnergy = frame[13];

  String stateDesc = "NO PRESENCE";
  if (targetState == 1) stateDesc = "MOVING TARGET ONLY";
  else if (targetState == 2) stateDesc = "STATIONARY TARGET ONLY";
  else if (targetState == 3) stateDesc = "MOVING + STATIONARY CROWD";

  Serial.print("[RADAR 24GHz] State: ");
  Serial.print(stateDesc);
  Serial.print("  |  Move: ");
  Serial.print(movingDist);
  Serial.print("cm (Energy: ");
  Serial.print(movingEnergy);
  Serial.print("%)  |  Stat: ");
  Serial.print(statDist);
  Serial.print("cm (Energy: ");
  Serial.print(statEnergy);
  Serial.println("%)");
}

void loop() {
  while (RadarSerial.available()) {
    uint8_t b = RadarSerial.read();

    if (bufIndex < 4) {
      if (b == FRAME_HEADER[bufIndex]) {
        buffer[bufIndex++] = b;
      } else {
        bufIndex = 0;
      }
    } else {
      buffer[bufIndex++] = b;
      if (bufIndex >= sizeof(buffer)) {
        bufIndex = 0; // Overflow safety
      }

      // Check for Tail
      if (bufIndex >= 4 &&
          buffer[bufIndex - 4] == FRAME_TAIL[0] &&
          buffer[bufIndex - 3] == FRAME_TAIL[1] &&
          buffer[bufIndex - 2] == FRAME_TAIL[2] &&
          buffer[bufIndex - 1] == FRAME_TAIL[3]) {
        
        parseRadarFrame(buffer, bufIndex);
        bufIndex = 0;
      }
    }
  }

  // Periodic heartbeat if no serial received (helps user check baud/wiring)
  static unsigned long lastRawCheck = 0;
  if (millis() - lastRawCheck > 3000) {
    lastRawCheck = millis();
    if (!RadarSerial.available() && bufIndex == 0) {
      Serial.println("[RADAR] Waiting for frames... Tip: Verify Radar TX -> Pin 16 and 5V power.");
    }
  }
}
