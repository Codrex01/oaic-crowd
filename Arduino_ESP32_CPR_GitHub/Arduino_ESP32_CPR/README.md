# CPR – Arduino ESP32 Firmware & Hardware Tests

This repository contains standalone Arduino sketches for the CPR (Crowd Pressure Radar) prototype.

## What's included

- `CPR_ESP32_Main/` – combined ESP32 firmware for the currently usable sensors and actuators:
  - HLK-LD2410C presence output + UART monitor
  - VL53L0X distance sensor
  - OLED display (SSD1306, I2C)
  - LED and buzzer status alerts
  - SG90 servo controlled manually from Serial Monitor
  - optional Wi-Fi connection status
- `tests/` – independent test sketches for each component.
- `tests/LoRa/` – SX1278/RA-02 SPI diagnostic and separate transmitter/receiver examples. These are **not** used by the main firmware while the modules are undetected.
- `tests/IR_Beam_Test/` – configurable input test template. Confirm the IR sensor's output type and voltage before connecting it.

## Important limitations

1. This firmware does **not** count people. The USB webcam + YOLO application runs on the computer and is a separate software component.
2. HLK-LD2410C reports human presence/motion information; it is not a reliable multi-person counter.
3. VL53L0X measures distance to a target; it does not measure crowd density or physical pressure.
4. A single IR beam detects a crossing, not reliable direction or individual count in all situations. Two spaced beams and controlled passage geometry are needed for direction.
5. The servo is kept under manual Serial Monitor control in the combined firmware. Test the mechanism on a bench; do not use it as a real safety barrier.
6. The LoRa sketches are diagnostic only. Do not connect RA-02 VCC to 5V. RA-02 is a 3.3V module. Attach a suitable 433 MHz antenna before transmitting.

## Hardware pin map used by the combined sketch

| Function | ESP32 pin |
|---|---:|
| I2C SDA (OLED + VL53L0X) | GPIO 21 |
| I2C SCL (OLED + VL53L0X) | GPIO 22 |
| HLK-LD2410C UART RX (ESP32 receives from sensor TX) | GPIO 16 |
| HLK-LD2410C UART TX (ESP32 sends to sensor RX) | GPIO 17 |
| HLK-LD2410C OUT presence signal | GPIO 32 |
| SG90 signal | GPIO 25 |
| Single status LED | GPIO 4 |
| Active buzzer control | GPIO 33 |

**GPIO 4 and GPIO 33 are defaults, not confirmed wiring from the project log.** Change `LED_PIN` and `BUZZER_PIN` in the sketch to match the actual wires. Use a transistor driver if the buzzer draws more current than an ESP32 GPIO can safely supply. Never drive a 5V buzzer directly from a GPIO.

### Power and wiring cautions

- RA-02 LoRa VCC: 3.3V only. Never 5V.
- HLK-LD2410C: use a regulated 5V supply capable of at least 200mA; common GND with ESP32. UART/OUT logic is 3.3V.
- SG90: use a suitable regulated 5V supply; connect supply GND to ESP32 GND. Do not power the servo from a GPIO or 3V3 pin.
- VL53L0X and OLED: I2C on GPIO 21/22. Confirm the breakout accepts the selected supply voltage.
- Do not connect or disconnect modules while powered.
- Check the exact board pin labels and use a multimeter to verify supply polarity before applying power.

## Arduino IDE setup

1. Install Arduino IDE 2.x.
2. In Boards Manager, install **esp32 by Espressif Systems**.
3. Select your exact ESP32 board and its USB/COM port.
4. Install these libraries from Library Manager:
   - `Adafruit VL53L0X`
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `ESP32Servo`
   - `LoRa` by Sandeep Mistry (only for LoRa tests)
5. Open each `.ino` from its own folder. Arduino expects the main `.ino` filename to match its containing folder name.
6. Upload one test at a time. Do not run multiple sketches simultaneously on the same ESP32.

## Combined firmware usage

Open `CPR_ESP32_Main/CPR_ESP32_Main.ino`.

- Open Serial Monitor at **115200 baud**.
- Type `O` + Enter to move the demo servo to the open position.
- Type `C` + Enter to move it to the closed position.
- Type `S` + Enter to print a sensor snapshot.
- Type `B` + Enter to test the buzzer briefly.
- Presence and distance readings are shown continuously.
- The buzzer/LED warning is based on a configurable **near-object distance threshold**, not crowd pressure.

The HLK UART monitor prints raw bytes for debugging. The `OUT` pin is used as the simple presence signal. The firmware does not decode the full LD2410C UART protocol.

## LoRa tests

The current reported SPI diagnostic returned `0xFF`, meaning the ESP32 did not receive a valid SX127x version response. The expected version register value for SX1278 is commonly `0x12`.

- First run `LoRa_SPI_Diagnostic`.
- Only proceed to TX/RX if the SPI diagnostic reads a valid version.
- Upload `LoRa_Receiver` to one ESP32 and `LoRa_Transmitter` to the other.
- Both modules must use compatible frequency settings. These examples use 433 MHz.
- If `0xFF` persists, inspect pin labels, NSS, MISO, reset, 3.3V under load, common ground, breadboard contacts and module orientation. Do not try 5V.

## GitHub

Create a GitHub repository and upload the contents of this folder. Do not upload Wi-Fi passwords, API keys, or real device secrets. Keep credentials in local configuration files that are excluded from Git.
