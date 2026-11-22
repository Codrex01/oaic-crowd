# CPR hardware inventory

Based on the project details shared so far. Verify physical models and wiring before powering.

## Reported available
- 2 x ESP32 development boards
- 2 x Ai-Thinker RA-02 SX1278 LoRa modules (433 MHz) – currently not detected by SPI test
- 1 x HLK-LD2410C 24 GHz presence radar – reported working
- 1 x VL53L0X ToF distance sensor – reported scanning; calibration needed
- 1 x SSD1306-compatible OLED (controller/address should be verified)
- 1 x SG90 servo – reported working
- LED indicator – reported working; exact pin/type should be checked
- Active buzzer/beeper – reported working; exact voltage/current/pin should be checked
- IR beam sensor – 12–24V relay-output product description reported; not configured
- USB webcam + computer running YOLO – separate from ESP32 firmware
- Regulated 5V 2A adapter, breadboards and wiring

## Not represented as completed
- LoRa link between nodes
- IR beam counting
- Validated occupancy count from IR
- Fused camera/radar/IR occupancy estimate
- Crowd-pressure measurement
