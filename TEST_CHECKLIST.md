# Crowd Pressure Radar: Test Script & Verification Checklist

Use this systematic checklist to verify each hardware, firmware, network, and dashboard subsystem prior to live demonstration or lab evaluations.

---

## 1. Pre-Flight Hardware Bench Checks

- [ ] **Power Isolation**: The SG90 servo and HLK-LD2450 radar are wired to the external 5V 2A power rail and **NOT** powered from the ESP32 3.3V pin.
- [ ] **Common Ground**: All GND pins (ESP32 GND, power supply GND, radar GND, servo GND, display GND) are bonded to a single shared bus.
- [ ] **UART Polarity**: ESP32 RX2 (GPIO16) connects to radar TX; ESP32 TX2 (GPIO17) connects to radar RX.
- [ ] **I2C Pullups**: OLED SDA (GPIO21) and SCL (GPIO22) are securely seated (SSD1306 module includes onboard 4.7kΩ pullups).
- [ ] **IR Beam Alignment**: Beam A (GPIO32) and Beam B (GPIO33) are aligned; blocking the beam drops GPIO pin voltage from 3.3V to 0V (GND).
- [ ] **Mock Mode Pin**: GPIO4 is tied to GND via switch/button for simulation, or left open (pulled high) for physical HLK-LD2450 UART operation.

---

## 2. Embedded Firmware Serial CLI Verification

Connect the ESP32 via USB and open the Serial Monitor at **115200 baud**. Execute the following commands:

| Command | Action / Input | Expected Serial Output | Verified |
| :--- | :--- | :--- | :---: |
| `status` | Type `status` + Enter | Dumps full JSON/table containing Device ID, radar driver name, current state, score, density, speed, gate angle, and WiFi state. | [ ] |
| `mock on` | Type `mock on` + Enter | `[CLI] Switched radar input to SYNTHETIC MOCK model.` Synthetic crowd scenario starts cycling. | [ ] |
| `mock off`| Type `mock off` + Enter | `[CLI] Switched radar input to PHYSICAL HLK-LD2450 UART.` ESP32 starts listening on UART2 for 0xAA 0xFF frames. | [ ] |
| `force red` | Type `force red` + Enter | Immediate state transition to RED: Red LED begins blinking at 2 Hz, buzzer sounds intermittent alarm, servo rotates to 90°. | [ ] |
| `force green` | Type `force green` + Enter | Immediate state transition to GREEN: Green LED lights solid, buzzer silences. Gate remains at 90° until 30s GREEN timer expires. | [ ] |
| `gate open` | Type `gate open` + Enter | Servo rotates to 0° (open). Logged with reason: `[SERVO GATE] Position -> 0 deg (OPEN)`. | [ ] |
| `gate close`| Type `gate close` + Enter | Servo rotates to 90° (closed). Logged with reason: `[SERVO GATE] Position -> 90 deg (CLOSED)`. | [ ] |

---

## 3. Sensor Frame & Analytics Pure Function Validation

- [ ] **Frame Header Sync**: Ensure 30-byte frames starting with `AA FF 03 00` and ending in `55 CC` are parsed. Check that `activeRadar->getDroppedFrames()` does not continually increase.
- [ ] **Sign Bit Decoding**: Verify that negative X coordinates (targets to the left of the radar) decode properly using the bit-15 sign magnitude rule (`// verify against the LD2450 datasheet`).
- [ ] **Density Calculation**: With 3 targets in a 24 m² zone, density computes to $3 / 24 = 0.125\text{ ppl/m}^2$.
- [ ] **Stop-and-Go Variance**: When target speeds fluctuate rapidly, the standard deviation over the 20-sample sliding window increases proportionally.
- [ ] **Clustering Trigger**: When two targets are within 500 mm ($<0.5\text{ m}$), `clusterPairs` increments to $\ge 1$.

---

## 4. State Machine Hysteresis & Timing Tests

- [ ] **AMBER Escalation Timing**: Hold score $\ge 50$ continuously. Verify state transitions to AMBER **only after 5.0 seconds** have elapsed (not prematurely).
- [ ] **RED Escalation Timing**: Hold score $\ge 75$ continuously. Verify state transitions to RED **only after 3.0 seconds** have elapsed.
- [ ] **AMBER De-escalation Hysteresis**: Lower score to 48 (below 50, but above 40). State must **remain AMBER**. Lower score below 40 ($50 - 10$) and hold for 10.0 seconds. Confirm state transitions to GREEN.
- [ ] **RED De-escalation Hysteresis**: Lower score to 70 (below 75, but above 65). State must **remain RED**. Lower score below 65 ($75 - 10$) and hold for 10.0 seconds. Confirm state transitions to AMBER.
- [ ] **Gate 30-Second Safety Hold**: Once gate barrier deploys to 90° on RED, return state to GREEN. Ensure gate **remains closed at 90° for a full 30 seconds** of continuous GREEN before automatically returning to 0°.

---

## 5. Network & Offline Ring-Buffer Resiliency

- [ ] **WiFi Telemetry Ingestion**: ESP32 POSTs JSON packets every 1.0 s to `/api/telemetry` with header `X-Device-Key: cpr-secret-key-2026`. Backend responds with HTTP `201 Created`.
- [ ] **Rate Limiting**: Attempting to send $>2$ requests/sec from the same device ID returns HTTP `429 Too Many Requests`.
- [ ] **Offline Buffering**: Disconnect the WiFi access point. Verify that the ESP32 queues up to 50 unsent telemetry records in its circular ring-buffer without crashing or running out of RAM.
- [ ] **Buffer Flush on Reconnect**: Reconnect WiFi. Verify that the ESP32 drains all buffered packets to the server with exponential backoff recovery.
- [ ] **Command Polling**: Dispatch `MANUAL_GATE_CLOSE` from the dashboard. Confirm that within 2.0 seconds the ESP32 receives the command on `GET /api/commands/:deviceId` and activates the servo.

---

## 6. Frontend Dashboard & Accessibility Audit

- [ ] **WCAG AA Contrast**: All state labels and badges (Green `#6EE7B7` on `#064E3B`, Amber `#FCD34D` on `#78350F`, Red `#FCA5A5` on `#7F1D1D`) exceed the 4.5:1 text contrast ratio requirement.
- [ ] **Multi-Modal Indicators**: Every alert state includes **Color + Unique Icon + Text Label** (never relying on color alone for color-blind accessibility).
- [ ] **Tablet Readability**: The large gate status decision text on the Overview card is easily readable from 2 meters away on a 10-inch tablet.
- [ ] **Disconnected Badge**: If no telemetry arrives for $\ge 5$ seconds, the live status indicator switches from "LIVE (WS)" to "DISCONNECTED / STALE (>5s)".
- [ ] **Confirmation Dialogs**: Clicking "DEPLOY GATE" or "RESET ZONE" opens a modal dialog requiring the operator to enter a justification reason before sending the command.
- [ ] **CSV Export**: In the Alerts tab, clicking "Export CSV" downloads a properly formatted `.csv` file with ISO timestamps, transition states, scores, and messages.
- [ ] **Automated Unit Tests**: Running `npm run test` passes all 6 Vitest test suites for state color tokens and risk formulation math.

