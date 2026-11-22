# CPR pin configuration and verification

## Combined firmware defaults

| Device | Signal | ESP32 |
|---|---|---:|
| HLK-LD2410C | TX -> ESP32 RX | GPIO 16 |
| HLK-LD2410C | RX <- ESP32 TX | GPIO 17 |
| HLK-LD2410C | OUT | GPIO 32 |
| OLED + VL53L0X | SDA | GPIO 21 |
| OLED + VL53L0X | SCL | GPIO 22 |
| SG90 | Signal | GPIO 25 |
| Status LED | Control | GPIO 4 (default; verify) |
| Active buzzer/driver | Control | GPIO 33 (default; verify) |

## LoRa RA-02 test wiring

| RA-02 pin | ESP32 |
|---|---:|
| VCC | 3V3 only |
| GND | GND |
| SCK | GPIO 18 |
| MISO | GPIO 19 |
| MOSI | GPIO 23 |
| NSS / CS | GPIO 13 |
| RESET | GPIO 27 |
| DIO0 | GPIO 26 |

## Conflicts to avoid

- The standalone LED test uses GPIO 4 by default to avoid I2C/SPI conflicts.
- LoRa uses GPIO 18/19/23 and GPIO 13/27/26. The main firmware does not initialize LoRa.
- OLED and VL53L0X share I2C GPIO 21/22.
- Do not connect an RGB LED channel to an I2C/SPI pin while those buses are in use.
- GPIO 34 is input-only and has no internal pull-up/down on classic ESP32.
- GPIO 6–11 are normally used by the ESP32 module's flash; avoid them.
