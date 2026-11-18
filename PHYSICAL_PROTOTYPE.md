# Physical Prototype Assembly & Mechanical Integration Guide

This document illustrates the complete physical prototype assembly for the **Crowd Pressure Radar**, detailing how all **16 hardware items from the bill of materials (BOM)** fit together into a demo-ready table-top corridor model or full-scale queue gate.

---

## 🏗️ 1. Physical Model Architecture & Layout

```
                                 [1. HLK-LD2450 24GHz mmWave Radar]
                                  Overhead on mast (20° down tilt)
                                             |
                                             v
  ===========================================\\=============================================
  CORRIDOR ENTRANCE                                                         CORRIDOR EXIT
  ==========================================================================================
   
   [3A. IR ENTRY BEAM]                                                  [3B. IR EXIT BEAM]
   (Emitter + Receiver)                                                 (Emitter + Receiver)
           |                                                                    |
           v                                                                    v
   +---------------+                                                    +---------------+
   | ENTRY POST    |                                                    | EXIT POST     |
   |               |                                                    |               |
   | [7. SG90]=====[ Barrier Arm (0° Open / 90° Closed) ]               |               |
   |               |                                                    |               |
   | [4. OLED]     |        ======> CROWD FLOW DIRECTION ======>        |               |
   | [5. RGB LEDs] |                                                    |               |
   | [6. Buzzer]   |                                                    |               |
   | [13. Enclosure|                                                    |               |
   |  with ESP32]  |                                                    |               |
   +---------------+                                                    +---------------+
  ==========================================================================================
   QUEUE BARRICADE WALL (Clear Acrylic / Aluminum Rails - Item 12)
  ==========================================================================================
           |
           +----> Routed to [8. Solderless Breadboard] + [10. Regulated 5V 2A DC Power Supply]
```

---

## 🔩 2. Component-by-Component Mechanical Integration

### 1. HLK-LD2450 mmWave Radar Sensor (24 GHz) – 1 Pc
* **Mounting Position**: Elevated on a vertical mast or mounting arm (30–40 cm height for the tabletop model; 2.0–2.5 m for full-scale installation) positioned above the entrance looking longitudinally down the corridor.
* **Angle**: Tilted **$15^\circ$ to $20^\circ$ downwards**. This orientation gives the $60^\circ$ azimuth beam an unobstructed line-of-sight along the 6-meter queue depth.
* **Wiring**: 4-wire ribbon cable routed down the interior of the mast:
  * `VCC` $\to$ External 5V Rail
  * `GND` $\to$ Common GND Rail
  * `TX` $\to$ ESP32 GPIO16 (RX2)
  * `RX` $\to$ ESP32 GPIO17 (TX2)

### 2. ESP32 DevKit Board (30-pin, CP2102) – 1 Pc
* **Mounting Position**: Housed securely inside the **3D-printed/acrylic project enclosure (Item 13)** affixed to the main entry stanchion post.
* **Role**: Orchestrates high-speed UART parsing (256,000 baud), real-time risk index calculation at 5 Hz, SSD1306 I2C display rendering, actuator PWM control, and Wi-Fi telemetry posting.

### 3. IR Break-Beam Sensors (Emitter + Receiver) – 2 Pairs
* **Pair A (Entrance Beam)**:
  * Emitter on left stanchion, receiver on right stanchion at waist height (10 cm above baseboard on model; 1.0 m on full scale).
  * Connected to **GPIO32 (`INPUT_PULLUP`)**. Breaking the invisible beam drops the line to GND and triggers the interrupt `isrEntryBeam()`, incrementing `entryCount`.
* **Pair B (Exit Beam)**:
  * Mounted at the far end of the corridor. Connected to **GPIO33 (`INPUT_PULLUP`)**.
  * Triggers `isrExitBeam()`, incrementing `exitCount`.
* **Net Flow Calculation**: Real-time corridor population is continuously computed as $\text{In-Zone Headcount} = \text{entryCount} - \text{exitCount}$.

### 4. 0.96-inch OLED Display (I2C, 128×64, SSD1306) – 1 Pc
* **Mounting Position**: Flush-mounted on the front faceplate of the stanchion enclosure at eye level.
* **Role**: Shows live state (`GREEN`, `AMBER`, `RED`), numerical score ($0-100$), graphical progress bar, crowd density, walking velocity, and gate state in an automatic 3-second carousel.

### 5. RGB LED Indicators (Red, Yellow, Green) – 1 Set
* **Mounting Position**: Vertical stack above the OLED display (miniature traffic beacon).
* **Connections**:
  * Green LED $\to$ GPIO25 via $220\,\Omega$ resistor
  * Yellow LED $\to$ GPIO26 via $220\,\Omega$ resistor
  * Red LED $\to$ GPIO27 via $220\,\Omega$ resistor
* **Behavior**:
  * **GREEN**: Green LED solid on.
  * **AMBER**: Yellow LED solid on.
  * **RED**: Red LED strobing at 2 Hz ($250\,\text{ms}$ on, $250\,\text{ms}$ off).

### 6. 5V Active Buzzer Module – 1 Pc
* **Mounting Position**: Inside the enclosure behind an acoustic grille.
* **Connection**: Signal pin to **GPIO14**, VCC/GND to 5V/GND.
* **Behavior**: Completely silent in GREEN; single $200\,\text{ms}$ alert chirp on entering AMBER; urgent pulsing acoustic siren during RED critical stall.

### 7. SG90 Micro Servo Motor (9g, 180°) – 1 Pc
* **Mounting Position**: Secured to the top of the entry post with the output horn attached to the **mechanical barrier boom arm (Item 12)**.
* **Connection**: PWM signal wire (Orange) to **GPIO13**. Power wires (Red/Brown) connected strictly to the **regulated 5V external power rail** and common ground.
* **Action**:
  * **$0^\circ$ (Vertical / Up)**: Flow open during GREEN and AMBER.
  * **$90^\circ$ (Horizontal / Closed)**: Deploys instantly upon entering RED to halt incoming queue crowd. Automatically re-opens only after 30 seconds of continuous GREEN.

### 8. Solderless Breadboard & 9. Jumper Wires
* **Mounting Position**: Base of the demonstration board or inside the lower electronics tray.
* **Role**: Central power distribution hub. Bridges the 5V and GND rails between the external power supply, ESP32, radar mast, servo motor, and IR sensors.

### 10. Regulated 5V 2A DC Power Supply
* **Role**: Powers the servo motor and HLK-LD2450 radar.
* **Critical Electrical Isolation**: Motors and radar draw dynamic peak currents ($>500\,\text{mA}$). Powering them from the ESP32 onboard 3.3V regulator causes immediate microcontroller brownout resets. The external 5V supply ensures rock-solid operational stability.

### 11. Resistors, Capacitors & Connecting Wires
* Three $220\,\Omega$ current-limiting resistors for the indicator LEDs; one $100\,\mu\text{F}$ electrolytic smoothing capacitor across the servo 5V rail to absorb inductive kickback.

### 12. Mini Corridor / Barricade Model Materials
* Clear laser-cut acrylic plates or aluminum miniature stanchions forming a $60\text{ cm} \times 15\text{ cm}$ corridor lane, equipped with a yellow/black striped boom gate.

### 13. Project Enclosure & Sensor Mounting Hardware
* Custom 3D-printed enclosure (STL files standard $85\text{ mm} \times 55\text{ mm} \times 35\text{ mm}$) featuring cutouts for the OLED screen, LED bezels, and buzzer sound holes.

---

## 🚀 3. Integration of the Optional Modular Components

The firmware and pin map are pre-configured to support all three optional items without code rewrites:

### 14. VL53L1X Time-of-Flight Distance Sensor (Optional)
* **Mounting Location**: Positioned on the gate barrier post pointed horizontally across the queue opening.
* **Function**: Micro-metric pinch-point monitoring. Detects if an individual tries to crawl under or push through the closed barrier arm.
* **Integration**: Shares the existing I2C bus on **GPIO21 (SDA)** and **GPIO22 (SCL)** alongside the OLED display (each sensor has distinct I2C addresses: OLED at `0x3C`, VL53L1X at `0x29`).

### 15. Additional ESP32 DevKit Board (Optional)
* **Mounting Location**: Positioned at the exit or midpoint of extended queue corridors ($>15\text{ meters}$).
* **Function**: Secondary zone monitoring node. Flashes identical firmware with `#define ZONE_ID "zone-corridor-b"`, streaming live telemetry to the dashboard for multi-zone queue heatmaps.

### 16. SX1278 LoRa Module (433 MHz) (Optional)
* **Mounting Location**: Inside the enclosure wired to ESP32 SPI pins:
  * `SCK` $\to$ GPIO18, `MISO` $\to$ GPIO19, `MOSI` $\to$ GPIO23, `NSS` $\to$ GPIO5, `RST` $\to$ GPIO15, `DIO0` $\to$ GPIO2.
* **Function**: Provides 3–5 kilometer long-range off-grid radio telemetry in remote temple pilgrimages or festival fields where commercial 4G/5G networks collapse due to cell-tower saturation.

