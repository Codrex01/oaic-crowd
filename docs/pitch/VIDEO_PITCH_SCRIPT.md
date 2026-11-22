# CROWD PRESSURE RADAR (CPR): 3-MINUTE VIDEO PITCH & DEMO SCRIPT
**Project:** Crowd Pressure Radar: Privacy-First Crowd Monitoring & Automated Crush Prevention  
**Target Duration:** 2 minutes 45 seconds to 3 minutes  
**Format:** Video presentation featuring speaker on camera, close-up physical hardware demo, and live dashboard screen recording.  

---

## VIDEO SHOOTING CHECKLIST & SETUP
* **On Your Desk:**
  * ESP32 breadboard with 0.96″ OLED, VL53L0X ToF sensor, and jumper wires connected.
  * Laptop running the **Live Dashboard** at `http://localhost:5173` and terminal showing backend logs.
* **Camera Angles:**
  1. **A-Roll (Face to Camera):** Confident, clear speaking tone.
  2. **B-Roll (Hardware Close-up):** Hands moving toward the VL53L0X sensor; OLED screen text clearly visible.
  3. **Screen Capture / Pan to Laptop:** Showing the live dashboard gauges shifting from Green to Amber to Red in real-time.

---

## TIMECODED SCRIPT & VISUAL CUES

### SCENE 1: THE HOOK & THE CRITICAL PROBLEM (0:00 – 0:35)
* **Visual:** Speaker on camera (or title slide on laptop, cutting to speaker).
* **Tone:** Urgent, authoritative, and mission-driven.

> **[SPEAKER]:**  
> "Every year, thousands of lives are endangered in sudden crowd collapses at high-density religious pilgrimages, transit stations, and festival corridors—from the tragic crush at Hathras that claimed 123 lives, to the Itaewon disaster in Seoul.
> 
> The forensic truth is that **people do not die from being trampled; they die standing up from compressive asphyxiation** when local crowd pressure exceeds 4,000 Newtons. 
> 
> Today, security relies on optical CCTV cameras. But in dense crowds, cameras suffer over 70% occlusion failure, are blinded by dust, smoke, and low temple lighting, and violate personal privacy. Worst of all, they are **reactive**—they only alert guards *after* the crush has already happened.
> 
> We built **Crowd Pressure Radar (CPR)**: a privacy-first, edge-computed multi-sensor system that detects crowd compression and queue stagnation **30 to 60 seconds before a physical crush occurs**, and automatically takes physical action."

---

### SCENE 2: THE INNOVATION & THE VL53L0X ToF SENSOR (0:35 – 1:15)
* **Visual:** Cut to close-up of the breadboard prototype on the desk. Point to the ESP32, the OLED, and the VL53L0X sensor module.

> **[SPEAKER]:**  
> "Our solution replaces intrusive video cameras with non-optical physical sensing:
> 
> At the core of our ingress-chute monitoring is the **VL53L0X Time-of-Flight (ToF) laser ranging sensor**. Unlike standard ultrasound or basic infrared, this sensor fires 940-nanometer VCSEL laser photons and measures their round-trip flight time at the speed of light with millimeter accuracy.
> 
> As devotees approach the gate barrier, the ToF sensor continuously tracks physical crowd compaction and leading-edge queue distance. 
> 
> Combined with our 24 GHz FMCW radar that measures walking velocity collapse, and photoelectric IR tripwires that meter net corridor headcount, the onboard **ESP32 microcontroller** calculates our proprietary **Helbing Crowd Turbulence and Risk Score** locally on the edge in under 50 milliseconds."

---

### SCENE 3: LIVE HARDWARE & DATA CENTER DEMONSTRATION (1:15 – 2:15)
* **Visual:** Split screen or camera over-the-shoulder showing the hardware in the foreground and the laptop dashboard in the background.

> **[SPEAKER]:**  
> "Let’s see the live system in action:
> 
> Right now, the corridor is clear. The VL53L0X ToF sensor reads open distance. Look at our physical OLED display: it reads **STATE: GREEN, Risk Score: 20**.
> 
> Now, watch what happens when crowd flow begins to compress:"

*(Action: Move your hand to approximately 80 cm in front of the sensor)*

> **[SPEAKER]:**  
> "A queue builds up. The ToF sensor instantly registers the compaction. The OLED immediately escalates to **AMBER — Risk Score 58**. Notice the response time: zero cloud lag, zero latency."

*(Action: Move your hand closer, under 40 cm from the sensor)*

> **[SPEAKER]:**  
> "Now, simulate a critical queue stall or barrier choke: the distance drops below 50 centimeters. 
> 
> Instantly, the system triggers **CRITICAL RED — Risk Score 85!**
> 
> On the physical edge node, the OLED flashes emergency warnings, the local acoustic siren triggers, and our mechanical servo turnstile gate physically closes to 90 degrees, halting incoming crowd ingress before compressive forces become fatal.
> 
> Now look at the laptop screen behind me: 
> Simultaneously, the telemetry packet has been transmitted via HTTP and WebSockets directly to our **centralized Data Center and Command Dashboard**. 
> The live dashboard updates instantly with our quad-coded alerts, real-time risk line, and zone status, allowing disaster management teams across the city to monitor every gate in real time."

---

### SCENE 4: OFF-GRID RESILIENCE & PRIVACY (2:15 – 2:40)
* **Visual:** Pan to the LoRa module / ESP32 antenna. Cut back to speaker.

> **[SPEAKER]:**  
> "In mass festivals like the Puri Rath Yatra or Kumbh Mela, mobile cellular networks inevitably crash under tower congestion. 
> 
> To guarantee life safety during complete telecom blackouts, CPR integrates the **Semtech RA-02 LoRa transceiver**. Operating on 433 MHz chirp spread spectrum, it broadcasts encrypted telemetry packets **3 to 5 kilometers off-grid** directly to the police command center with zero reliance on cellular data or Wi-Fi.
> 
> Most importantly: **Zero facial recognition. Zero cameras. Zero personal identifiable information.** CPR is 100% compliant with India's DPDP Act 2023 and the global GDPR."

---

### SCENE 5: ECONOMICS & CLOSING CALL TO ACTION (2:40 – 3:00)
* **Visual:** Speaker looking directly into the camera. Confident, smiling, decisive.

> **[SPEAKER]:**  
> "A typical CCTV AI camera setup costs upwards of ₹8.4 Lakhs ($10,000 USD) for 10 gates with heavy GPU servers. 
> 
> Our complete Crowd Pressure Radar node costs under **₹2,500 INR ($27 USD)**—a **97% cost reduction**, making mass deployment across hundreds of temple gates and railway footbridges feasible tomorrow.
> 
> Safety is not about watching people; it is about protecting them before the pressure builds.
> 
> We are team Crowd Pressure Radar. Thank you."

---

## PRO-TIPS FOR RECORDING
1. **Lighting:** Ensure good desk lighting so the OLED screen text is readable on camera without exposure blowout (lower your phone's camera exposure slightly if the OLED is too bright).
2. **Audio:** Use your phone's wired earphones or collar mic if available so your voice is crisp and clear.
3. **Smooth Demo Hand Movement:** When demonstrating the distance threshold, move your hand steadily (Open $\rightarrow$ 80cm $\rightarrow$ 30cm) and hold it for 3–4 seconds at each step so the viewer can see the OLED and web dashboard update!
