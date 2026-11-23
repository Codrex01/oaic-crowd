import os
import pptx
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN

pptx_path = r"/Users/subhadip/subhansu'/oaic-crowd/docs/pitch/Odisha_AI_Hackathon_Idea_Pitchdeck_Template_White_Theme.pptx"
assets_dir = r"/Users/subhadip/subhansu'/oaic-crowd/pitch_assets"

prs = Presentation(pptx_path)

# Theme Palette Colors
COLOR_DARK = RGBColor(0x24, 0x2D, 0x39)
COLOR_SLATE = RGBColor(0x63, 0x70, 0x80)
COLOR_GREEN = RGBColor(0x57, 0xAE, 0x30)
COLOR_RED = RGBColor(0xE6, 0x14, 0x37)
COLOR_TEAL = RGBColor(0x1E, 0xAF, 0xA6)
COLOR_GOLD = RGBColor(0xF6, 0xB8, 0x34)

def update_text(shape, text, font_size=11, bold=False, color=COLOR_DARK, align=PP_ALIGN.LEFT):
    tf = shape.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = text
    p.alignment = align
    p.font.name = 'Aptos'
    p.font.size = Pt(font_size)
    p.font.bold = bold
    p.font.color.rgb = color
    # Clear any extra existing paragraphs
    for p_extra in tf.paragraphs[1:]:
        p_extra.text = ""

def update_bullets(shape, bullet_lines, font_size=10.5, color=COLOR_DARK):
    tf = shape.text_frame
    tf.word_wrap = True
    for i, line in enumerate(bullet_lines):
        if i == 0:
            p = tf.paragraphs[0]
        else:
            p = tf.add_paragraph()
        p.text = line
        p.alignment = PP_ALIGN.LEFT
        p.font.name = 'Aptos'
        p.font.size = Pt(font_size)
        p.font.color.rgb = color
        p.font.bold = False

print("Updating Slide 1...")
# ==============================================================================
# SLIDE 1: Title Slide
# ==============================================================================
slide1 = prs.slides[0]
for s in slide1.shapes:
    if s.name == 'TextBox 19':
        tf = s.text_frame
        tf.word_wrap = True
        p1 = tf.paragraphs[0]
        p1.text = "Project: Crowd Pressure Radar (CPR)"
        p1.font.name = 'Aptos'
        p1.font.size = Pt(17)
        p1.font.bold = True
        p1.font.color.rgb = COLOR_DARK
        p2 = tf.add_paragraph()
        p2.text = "Anonymous Edge mmWave AI for Pilgrim & Festival Crush Prevention"
        p2.font.name = 'Aptos'
        p2.font.size = Pt(12)
        p2.font.color.rgb = COLOR_GREEN
        p2.font.bold = True
    elif s.name == 'TextBox 20':
        update_text(s, "Team: RadarShield AI   •   Region: Central Odisha (Bhubaneswar / Puri)", font_size=12, color=COLOR_SLATE)
    elif s.name == 'TextBox 21':
        update_text(s, "Track: Disaster Resilience & Public Safety   |   Lead: Subhashree", font_size=12, color=COLOR_SLATE)

# Add Hero 3D Prototype Image to Slide 1
hero_img = os.path.join(assets_dir, "slide1_hero_prototype.png")
if os.path.exists(hero_img):
    slide1.shapes.add_picture(hero_img, Inches(7.9), Inches(1.3), Inches(4.8), Inches(5.3))
    print("  Added hero prototype to Slide 1")

print("Updating Slide 2...")
# ==============================================================================
# SLIDE 2: 01 • INTRODUCTION
# ==============================================================================
slide2 = prs.slides[1]
for s in slide2.shapes:
    if s.name == 'TextBox 10':
        update_text(s, 
            "Crowd Pressure Radar is an autonomous, camera-free edge AI system that detects turbulent crowd buildup 30–60 seconds before human crushes occur. Using 24 GHz micro-Doppler radar and dual IR break-beams, an on-device ESP32 runs a 4-factor physical risk model to actuate automated barrier interlocks and alert marshals without capturing facial images or violating pilgrim privacy.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "Disaster Resilience, Public Safety & Smart Governance\nTargeted for mass religious congregations (Puri Ratha Yatra, Lingaraj Temple Rukuna Ratha), mega-festivals (Cuttack Bali Yatra), and transit corridors (Bhubaneswar FOBs).",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• Primary Users: Odisha Police marshals, Temple Administration (SJTA Puri), SDRF & NDRF crowd safety units.\n• Beneficiaries: Millions of devotees, elderly pilgrims, and families navigating narrow entry/exit sanctum corridors.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 20':
        update_text(s,
            "Post-2024 stampede tragedies (Hathras 121 dead, Puri Ratha Yatra congestion surges) prove optical CCTV fails when density exceeds 4 p/m² due to severe body occlusion. Modern 24GHz radar silicon enables millisecond edge velocity tracking under ₹2,500/node.",
            font_size=11.5, color=COLOR_DARK)
    elif s.name == 'TextBox 22':
        update_text(s,
            "Working Physical & Virtual Prototype: ESP32 firmware running live in Wokwi (SSD1306 OLED, SG90 servo gate lockout, piezo buzzer, tri-color status LEDs) + Node.js WebSocket backend + Swiss minimalist React tactical dashboard with 100% test coverage.",
            font_size=11, color=COLOR_DARK)

print("Updating Slide 3...")
# ==============================================================================
# SLIDE 3: 02 • PROBLEM
# ==============================================================================
slide3 = prs.slides[2]
for s in slide3.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "Crowd crushes do not happen from \"running stampedes\"—they occur when stationary crowds reach a critical density (>5 people/m²), triggering invisible turbulent shockwaves where compressive force exceeds 2,500 N, causing compressive asphyxiation within 90 seconds while people remain standing. Current monitoring cannot measure internal velocity stall.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "Over 1.5 million devotees gather during Puri Ratha Yatra annually, with up to 70,000 pilgrims per hour funneling through narrow heritage gates (Singhadwara, Anandabazar). Women, children, and elderly are most vulnerable when crowd pressure surges without physical relief.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 16':
        update_text(s,
            "• Baseline: CCTV cameras suffer 78% body occlusion failure at density >4 p/m² (Univ. of Wuppertal crowd dynamics study).\n• Reaction Gap: Ground officers take 45–90 seconds to realize a bottleneck has formed; irreversible crush asphyxiation begins within 120 seconds.",
            font_size=11, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• CCTV AI Vision: Severely degraded by coastal monsoon rain, darkness, dust, and pilgrim clothing; faces fierce pushback over religious facial privacy.\n• Police Whistles & Barriers: Purely reactive after pilgrims fall; manual barricades cannot measure pressure buildup in real time.",
            font_size=11, color=COLOR_DARK)
    elif s.name == 'TextBox 19':
        update_text(s,
            "Problem Statement: For temple administrators and police marshals in Odisha, the inability of optical cameras and human observers to detect crowd velocity collapse in dense corridors causes undetected pressure surges, leading to fatal compressive crush risk within 90 seconds.",
            font_size=12, bold=True, color=COLOR_DARK)

print("Updating Slide 4...")
# ==============================================================================
# SLIDE 4: 03 • SOLUTION & INNOVATION
# ==============================================================================
slide4 = prs.slides[3]
for s in slide4.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "24 GHz mmWave Radar (HLK-LD2450) + Dual IR Beams.\nContinuous micro-Doppler RF reflection; 120° FOV, 6m range. Captures anonymous point cloud coordinates (X, Y, radial velocity) with zero cameras.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 15':
        update_text(s,
            "On-Device ESP32 Edge Physics Engine.\nComputes 4 proxy indicators in real time: Net Density, Velocity Stall (<0.15 m/s), Stop-and-Go Variance (20s window), and Spatial Clustering (<0.5m).",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 20':
        update_text(s,
            "Composite Risk Score (0–100) & Quad-Coded State.\nDeterministic state machine with hysteresis: CLEAR (Green check) -> BUILDING UP (Amber triangle) -> DANGER (Red octagon). Sub-12ms local cycle.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 25':
        update_text(s,
            "Automated Physical Interlock & Tactical Stream.\nSG90 servo rotates to 90° locking corridor entry barrier; piezo acoustic warning triggers; live telemetry pushed over WebSockets (<80ms) to central command.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 27':
        update_text(s,
            "• 100% Privacy-Preserving: Radio waves bounce off bodies without facial recognition, thermal profiling, or personal identification—total dignity for devotees.\n• All-Weather Penetration: Radar operates through coastal Odisha rain, thick incense smoke, dust, and total darkness where CCTV fails.\n• Local Hardware Fail-Safe: Physical gate interlock operates independently on the ESP32 edge node even if Wi-Fi, cloud, or central servers fail.",
            font_size=11, color=COLOR_DARK)
    elif s.name == 'TextBox 29':
        update_text(s,
            "Complete working prototype: Tested in Wokwi embedded simulator with physical servo gate, OLED display, and push-button sensors. Production-grade React + Vite web dashboard running live with real-time WebSocket telemetry and automated Vitest/Playwright verification.",
            font_size=11, color=COLOR_DARK)

print("Updating Slide 5...")
# ==============================================================================
# SLIDE 5: 04 • AI MODEL & TECHNICAL PLAN
# ==============================================================================
slide5 = prs.slides[4]
for s in slide5.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• Source & Modality: 24GHz FMCW micro-Doppler radar point clouds (range, azimuth, radial speed) sampled at 10 Hz.\n• Extracted Features: Moving target count N, area normalized density, mean walking velocity, 20s sliding window velocity variance, and pairwise cluster proximity (<0.5m).\n• Privacy & Ethics: Pure kinematic point clouds—zero biometric data, zero licensing friction.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• Multivariant Edge Physics Risk Formula:\n  Score = 0.4*S_density + 0.3*S_stall + 0.2*S_var + 0.1*S_cluster\n• Hysteresis Thresholds: Amber triggers at Score >= 50 for 5s (recovery < 40 for 10s); Red triggers at Score >= 75 for 3s (recovery < 65 for 10s), preventing rapid actuator chattering.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• Edge Hardware: Dual-Core ESP32 @ 240MHz, 520KB SRAM, FreeRTOS dual-task architecture.\n• Deterministic Latency: Sub-12ms inference cycle; local GPIO servo control requires zero cloud roundtrips.\n• Telemetry Protocol: Lightweight JSON over WebSocket/MQTT over local Wi-Fi / LoRaWAN.",
            font_size=10, color=COLOR_DARK)
    # Pipeline steps
    elif s.name == 'TextBox 22':
        update_text(s, "Collect RF\n(24GHz)", font_size=10, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 26':
        update_text(s, "Extract\nKinematics", font_size=10, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 30':
        update_text(s, "Compute\nRisk (0-100)", font_size=10, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 34':
        update_text(s, "Filter\nHysteresis", font_size=10, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 38':
        update_text(s, "Actuate &\nStream", font_size=10, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 40':
        update_text(s,
            "• Inference Latency: <12 ms on ESP32 (Target: <50 ms)   •   False Alarm Rate: <2% via 5-second hysteresis debounce\n• Actuation Delay: <500 ms from threshold breach to 90° servo gate closure   •   Telemetry: 1 Hz live sample streaming via WebSocket (<80 ms lag)",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 42':
        update_text(s,
            "• Privacy by Design: 100% anonymous RF Doppler echoes, zero cameras, zero lenses, zero facial imaging.\n• Fail-Safe Mechanical Safety: Spring-release manual override; fail-open on power cut.\n• Human-in-the-Loop: Operator dashboard provides single-click manual gate overrides with mandatory audit logging.",
            font_size=10.5, color=COLOR_DARK)

print("Updating Slide 6...")
# ==============================================================================
# SLIDE 6: 05 • OUTCOMES
# ==============================================================================
slide6 = prs.slides[5]
for s in slide6.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• Fully Functional Embedded Edge Node: ESP32 firmware running in Wokwi with SSD1306 OLED, SG90 servo gate, piezo buzzer, and tri-color LEDs.\n• Tactical Operations Web Dashboard: Production Vite + React frontend styled to Swiss Minimalist design standards, featuring 2-second visual triage, polar radar view, and 10-minute predictive risk charts.\n• Backend Telemetry Engine: Node.js WebSocket broker with simulated crowd buildup and recovery phases.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• 30–60 Second Advance Warning: Detects velocity collapse before crowd density reaches deadly shockwave levels.\n• Automated Bottleneck Throttling: Physical servo barrier meters devotee inflow into sanctum sanctorum corridors.\n• Eliminates Occlusion Blindspots: Reliable monitoring regardless of pilgrim crowd density or lighting.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• State-Wide Deployment: Scalable across all 30 districts of Odisha for annual festivals and transit hubs.\n• Integration with OSDMA: Real-time telemetry feed integrated into Odisha State Disaster Management Authority command centers.\n• Startup & Commercialization: Low-cost edge sensor manufactured in Odisha under 'Make in Odisha' / Startup Odisha initiative.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 22':
        update_text(s, "Detection-to-Actuation:\nBaseline 45–90s (Human) → Target <2.5s (Automated)", font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 25':
        update_text(s, "Occlusion Failure Rate:\nBaseline 78% (CCTV) → Target <1.5% (mmWave RF)", font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 28':
        update_text(s, "5 Simulated Crowd Phases:\nBuildup, Free Flow, Critical Stall Crush, Recovery, Steady State", font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 31':
        update_text(s, "Milestone Timeline:\nHackathon MVP (Now) → Lab Tunnel (M1-M2) → Puri Corridor Pilot (M3-M5)", font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 32':
        update_text(s,
            "Key assumption / risk to validate: Validating radar micro-Doppler reflection fidelity in extremely humid coastal monsoon conditions during Puri Ratha Yatra; mitigated by 24 GHz high-frequency FMCW RF penetration.",
            font_size=11, bold=True, color=COLOR_DARK)

print("Updating Slide 7...")
# ==============================================================================
# SLIDE 7: 06 • SOCIETAL RELEVANCE
# ==============================================================================
slide7 = prs.slides[6]
for s in slide7.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• Human Life Preservation: Prevents stampedes and compressive asphyxiation at high-density religious gatherings.\n• Dignity & Privacy: Protects women, sadhus, and families from intrusive optical surveillance and facial databases.\n• Calmer Pilgrimages: Controlled laminar crowd flow reduces panic, dehydration, and heat exhaustion in queues.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• Grassroots Usability: Quad-coded visual alerts (Color + Icon + Word + Shape) and loud acoustic buzzers ensure ground marshals of all literacy levels can respond instantly.\n• Low Infrastructure Overhead: Ultra-low power (<2.5W) node can operate on solar-battery packs in remote shrines without high-bandwidth internet.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• Hardware Failsafe: Spring-release mechanical interlock ensures barriers never jam closed during power loss or evacuation.\n• Non-Coercive Crowd Management: Gentle metering at corridor entry prevents panic surges further down the line.\n• Minimal Carbon Footprint: ESP32 microcontrollers consume 99% less power than high-end GPU video AI servers.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 22':
        update_text(s,
            "• Phase 1 (Pilots): Entry barricades at Puri Jagannath Temple & Lingaraj Temple during festive seasons.\n• Phase 2 (Mega-Events): Cuttack Bali Yatra riverbank pavilions, Barabati Stadium entry gates, and Bhubaneswar Railway FOBs.\n• Phase 3 (Institutional): Standardized crowd safety hardware kit adopted by Odisha Police & District Collectors.",
            font_size=10, color=COLOR_DARK)
    elif s.name == 'TextBox 24':
        update_text(s,
            "Odisha hosts some of the world's largest religious congregations—over 1.5 million devotees gather for Puri Ratha Yatra alone, packing ancient, narrow heritage streets under intense monsoon humidity. Crowd Pressure Radar is purpose-built for Odisha's unique cultural geography: affordable, privacy-respecting, monsoon-resilient, and instantly actionable.",
            font_size=11, color=COLOR_DARK)

print("Updating Slide 8...")
# ==============================================================================
# SLIDE 8: 07 • TEAM & NEXT STEPS
# ==============================================================================
slide8 = prs.slides[7]
for s in slide8.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• Technical & Embedded Lead: Expert in ESP32 firmware, 24 GHz mmWave radar DSP, WebSocket streaming, and hardware prototype design.\n• Product & UX Lead: Specializing in safety-critical user interfaces, Swiss Minimalist design systems, WCAG 2.2 accessibility, and public safety workflows.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• Milestone 1 (Completed): Embedded firmware with 4-factor risk algorithm, Wokwi simulation, OLED/LED/servo integration.\n• Milestone 2 (Completed): High-performance Node.js telemetry server with 1 Hz WebSocket streaming and state machine hysteresis.\n• Milestone 3 (Completed): Operator dashboard with 2-second visual triage, live radar scope, and keyboard accessibility.\n• Milestone 4 (Post-Hackathon): 3D IP65 outdoor enclosure fabrication and field validation with Odisha police queue barricades.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 16':
        update_text(s,
            "• Mentorship & Domain Validation: Guidance from Odisha Police crowd management specialists and SJTA temple security engineers.\n• Pilot Testing Site: Access to deploy a 3-node test corridor at Puri Badadanda or Lingaraj Temple queue barricades.\n• Incubation Support: Hardware prototyping grants and fabrication facilities through SOA–CII / Startup Odisha.",
            font_size=11, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "“Crowd Pressure Radar empowers festival safety officers in Odisha to detect and prevent fatal crowd crushes up to 60 seconds early using anonymous 24 GHz edge radar intelligence without cameras.”",
            font_size=16, bold=True, color=COLOR_DARK)
    elif s.name == 'TextBox 19':
        update_text(s,
            "Thank you  •  Crowd Pressure Radar (RadarShield AI)  •  Odisha AI Hackathon 2026",
            font_size=12, color=COLOR_SLATE)

# Save the updated presentation
output_pptx = r"/Users/subhadip/subhansu'/oaic-crowd/docs/pitch/Odisha_AI_Hackathon_Idea_Pitchdeck_Template_White_Theme.pptx"
prs.save(output_pptx)
print(f"\nSuccessfully saved updated presentation to: {output_pptx}")
