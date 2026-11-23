import os
import pptx
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN

pptx_path = r"C:\Users\subha\Downloads\crowd-pressure-radar\Odisha_AI_Hackathon_Idea_Pitchdeck_Template_White_Theme.pptx"
assets_dir = r"C:\Users\subha\Downloads\crowd-pressure-radar\pitch_assets"

prs = Presentation(pptx_path)

COLOR_DARK = RGBColor(0x24, 0x2D, 0x39)
COLOR_SLATE = RGBColor(0x63, 0x70, 0x80)
COLOR_GREEN = RGBColor(0x57, 0xAE, 0x30)
COLOR_RED = RGBColor(0xE6, 0x14, 0x37)
COLOR_TEAL = RGBColor(0x1E, 0xAF, 0xA6)
COLOR_GOLD = RGBColor(0xF6, 0xB8, 0x34)

def update_text(shape, text, font_size=10, bold=False, color=COLOR_DARK, align=PP_ALIGN.LEFT):
    tf = shape.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = text
    p.alignment = align
    p.font.name = 'Aptos'
    p.font.size = Pt(font_size)
    p.font.bold = bold
    p.font.color.rgb = color
    for p_extra in tf.paragraphs[1:]:
        p_extra.text = ""

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

# Adjust hero prototype picture on Slide 1 so it sits cleanly above hosting partners
# Remove existing pictures if any
for s in list(slide1.shapes):
    if s.shape_type == pptx.enum.shapes.MSO_SHAPE_TYPE.PICTURE and "slide1_hero" in str(getattr(s, 'image', '')):
        # python-pptx doesn't easily delete, but we can overwrite or place cleanly
        pass

hero_img = os.path.join(assets_dir, "slide1_hero_prototype.png")
if os.path.exists(hero_img):
    slide1.shapes.add_picture(hero_img, Inches(7.9), Inches(1.1), Inches(4.7), Inches(4.3))

# ==============================================================================
# SLIDE 2: 01 • INTRODUCTION
# ==============================================================================
slide2 = prs.slides[1]
for s in slide2.shapes:
    if s.name == 'TextBox 10':
        update_text(s, 
            "Crowd Pressure Radar is an autonomous, camera-free edge AI system that detects turbulent crowd buildup 30–60s before crushes occur. Using 24GHz micro-Doppler radar and dual IR beams, an ESP32 runs a 4-factor physical risk model to actuate automated barrier interlocks without violating pilgrim privacy.",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "Disaster Resilience, Public Safety & Smart Governance.\nEngineered for mass gatherings (Puri Ratha Yatra, Lingaraj Temple Rukuna Ratha), mega-festivals (Cuttack Bali Yatra), and urban transit corridors (Bhubaneswar FOBs).",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• Primary Operators: Odisha Police marshals, Shree Jagannath Temple Administration (SJTA Puri), SDRF/NDRF.\n• Beneficiaries: Millions of devotees, elderly pilgrims, and families in narrow sanctum queues.",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 20':
        update_text(s,
            "Post-2024 stampedes (Hathras 121 dead, Puri Ratha Yatra congestion surges) prove optical CCTV fails when density exceeds 4 p/m² due to severe body occlusion. Modern 24GHz radar silicon enables millisecond edge velocity tracking under ₹2,500/node.",
            font_size=11, color=COLOR_DARK)
    elif s.name == 'TextBox 22':
        update_text(s,
            "Working Physical & Virtual Prototype: ESP32 firmware running live in Wokwi (SSD1306 OLED, SG90 servo gate lockout, piezo buzzer, tri-color status LEDs) + Node.js WebSocket backend + Swiss minimalist React tactical dashboard with 100% test coverage.",
            font_size=10.5, color=COLOR_DARK)

# ==============================================================================
# SLIDE 3: 02 • PROBLEM
# ==============================================================================
slide3 = prs.slides[2]
for s in slide3.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "Crowd crushes occur when stationary crowds exceed critical density (>5 p/m²), triggering turbulent shockwaves where compressive chest force exceeds 2,500 N, causing compressive asphyxiation within 90 seconds while people remain standing. Current systems cannot measure internal velocity stall.",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "Over 1.5 million devotees gather during Puri Ratha Yatra annually, with up to 70,000 pilgrims/hour funneling through narrow heritage gates (Singhadwara, Anandabazar). Women, children, and elderly are most vulnerable when crowd pressure surges without physical relief.",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 16':
        update_text(s,
            "• Baseline: CCTV cameras suffer 78% body occlusion failure at density >4 p/m² (Univ. of Wuppertal crowd dynamics study).\n• Reaction Gap: Ground officers take 45–90 seconds to realize a bottleneck has formed; irreversible crush asphyxiation begins within 120 seconds.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• CCTV AI Vision: Severely degraded by coastal monsoon rain, darkness, dust, and pilgrim clothing; faces fierce pushback over religious facial privacy.\n• Police Whistles & Barriers: Purely reactive after pilgrims fall; manual barricades cannot measure pressure buildup in real time.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 19':
        update_text(s,
            "Problem Statement: For temple administrators and police marshals in Odisha, the inability of optical cameras and human observers to detect crowd velocity collapse in dense corridors causes undetected pressure surges, leading to fatal compressive crush risk within 90 seconds.",
            font_size=11.5, bold=True, color=COLOR_DARK)

# ==============================================================================
# SLIDE 4: 03 • SOLUTION & INNOVATION
# ==============================================================================
slide4 = prs.slides[3]
for s in slide4.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "24GHz mmWave Radar (HLK-LD2450) + Dual IR Beams.\nContinuous micro-Doppler RF reflection; 120° FOV, 6m range. Captures anonymous point cloud coordinates (X, Y, radial velocity) with zero cameras.",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 15':
        update_text(s,
            "On-Device ESP32 Edge Physics Engine.\nComputes 4 proxy indicators in real time: Net Density, Velocity Stall (<0.15 m/s), Stop-and-Go Variance (20s window), and Spatial Clustering (<0.5m).",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 20':
        update_text(s,
            "Composite Risk Score (0–100) & Quad State.\nDeterministic state machine with hysteresis: CLEAR (Green) -> BUILDING UP (Amber) -> DANGER (Red). Sub-12ms local cycle.",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 25':
        update_text(s,
            "Automated Interlock & Tactical Stream.\nSG90 servo rotates to 90° locking corridor entry barrier; piezo acoustic warning triggers; live telemetry pushed via WebSockets (<80ms).",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 27':
        # Shorten width to 7.0 in to leave right side open for flowchart graphic
        s.width = Inches(7.0)
        update_text(s,
            "• 100% Privacy-Preserving: Radio waves bounce off bodies without facial recognition or personal identification—total dignity for devotees.\n• All-Weather Penetration: Radar operates through coastal Odisha rain, thick incense smoke, dust, and total darkness where CCTV fails.\n• Local Hardware Fail-Safe: Physical gate interlock operates independently on the ESP32 edge node even if Wi-Fi or central servers fail.",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 28':
        pass
    elif s.name == 'TextBox 29':
        s.width = Inches(5.8)
        update_text(s,
            "Complete working prototype: Tested in Wokwi embedded simulator with physical servo gate, OLED display, and push-button sensors. Production-grade React + Vite web dashboard running live with real-time WebSocket telemetry and automated Vitest/Playwright verification.",
            font_size=9.5, color=COLOR_DARK)

# Add Technology Benchmark Graphic to Slide 4 right side
bench_img = os.path.join(assets_dir, "technology_benchmark.png")
if os.path.exists(bench_img):
    slide4.shapes.add_picture(bench_img, Inches(8.0), Inches(4.45), Inches(4.7), Inches(2.4))
    print("  Added technology benchmark to Slide 4")

# ==============================================================================
# SLIDE 5: 04 • AI MODEL & TECHNICAL PLAN
# ==============================================================================
slide5 = prs.slides[4]
for s in slide5.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• 24GHz FMCW Radar Point Clouds: Sampled at 10 Hz (range, azimuth, radial velocity).\n• Extracted Features: Target count N, normalized density, mean velocity, 20s variance, cluster pairs.\n• 100% Anonymous: Zero biometric PII, zero facial data.",
            font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• Multivariant Risk: Score = 0.4*S_density + 0.3*S_stall + 0.2*S_var + 0.1*S_cluster.\n• Hysteresis Debounce: Amber triggers >=50 (5s), clears <40 (10s); Red triggers >=75 (3s), clears <65 (10s) to prevent actuator chattering.",
            font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• Edge Hardware: Dual-Core ESP32 @ 240MHz, 520KB SRAM, FreeRTOS dual-task loop.\n• Deterministic Latency: Sub-12ms inference; local GPIO servo control requires zero cloud roundtrips.\n• Connectivity: WebSocket/MQTT over local Wi-Fi / LoRaWAN.",
            font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 22':
        update_text(s, "Collect RF\n(24GHz)", font_size=9, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 26':
        update_text(s, "Extract\nKinematics", font_size=9, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 30':
        update_text(s, "Compute\nRisk (0-100)", font_size=9, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 34':
        update_text(s, "Filter\nHysteresis", font_size=9, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 38':
        update_text(s, "Actuate &\nStream", font_size=9, color=COLOR_DARK, align=PP_ALIGN.CENTER)
    elif s.name == 'TextBox 40':
        s.width = Inches(5.3)
        update_text(s,
            "• Inference Latency: <12 ms on ESP32 (Target: <50 ms)   •   False Alarm Rate: <2% via 5-second hysteresis debounce\n• Actuation Delay: <500 ms from threshold breach to 90° servo gate closure   •   Telemetry: 1 Hz live sample streaming via WebSocket (<80 ms lag)",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 42':
        s.width = Inches(5.3)
        update_text(s,
            "• Privacy by Design: 100% anonymous RF Doppler echoes, zero cameras, zero lenses, zero facial imaging.\n• Fail-Safe Mechanical Safety: Spring-release manual override; fail-open on power cut.\n• Human-in-the-Loop: Operator dashboard provides single-click manual gate overrides with mandatory audit logging.",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 44':
        # Remove or hide overlapping duplicate footer
        s.text_frame.text = ""

# Add Weights Donut Chart to Slide 5 right side
donut_img = os.path.join(assets_dir, "weights_donut.png")
if os.path.exists(donut_img):
    slide5.shapes.add_picture(donut_img, Inches(8.1), Inches(5.2), Inches(4.6), Inches(2.0))
    print("  Added weights donut chart to Slide 5")

# ==============================================================================
# SLIDE 6: 05 • OUTCOMES
# ==============================================================================
slide6 = prs.slides[5]
for s in slide6.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• Embedded Node: ESP32 firmware running in Wokwi (SSD1306 OLED, SG90 servo gate, piezo buzzer, tri-color LEDs).\n• Tactical Web Dashboard: Vite + React frontend styled to Swiss Minimalist standards with live polar radar & 10-min risk trends.\n• Telemetry Engine: Node.js WebSocket broker simulating 5 crowd buildup & recovery phases.",
            font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• 30–60s Advance Warning: Detects velocity collapse before crowd density reaches deadly shockwave levels.\n• Automated Throttling: Physical servo barrier meters devotee inflow into sanctum sanctorum corridors.\n• Zero Occlusion Blindspots: Reliable monitoring regardless of crowd density or lighting.",
            font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• State-Wide Scalability: Deployable across all 30 districts of Odisha for annual festivals and transit hubs.\n• OSDMA Integration: Direct telemetry feed to Odisha State Disaster Management Authority centers.\n• Startup Pathway: Low-cost edge sensor manufactured in Odisha under 'Make in Odisha'.",
            font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 22':
        update_text(s, "Detection-to-Actuation:\n45–90s (Human) → <2.5s (Automated)", font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 25':
        update_text(s, "Occlusion Failure Rate:\n78% (CCTV) → <1.5% (mmWave RF)", font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 28':
        update_text(s, "5 Simulated Phases:\nBuildup, Free Flow, Stall, Recovery, Steady", font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 31':
        update_text(s, "Milestones:\nHackathon MVP → Lab Tunnel → Puri Pilot", font_size=8.5, color=COLOR_DARK)
    elif s.name == 'TextBox 32':
        update_text(s,
            "Key assumption / risk to validate: Validating radar micro-Doppler reflection fidelity in extremely humid coastal monsoon conditions during Puri Ratha Yatra; mitigated by 24 GHz high-frequency FMCW RF penetration.",
            font_size=10, bold=True, color=COLOR_DARK)

# ==============================================================================
# SLIDE 7: 06 • SOCIETAL RELEVANCE
# ==============================================================================
slide7 = prs.slides[6]
for s in slide7.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• Human Life Preservation: Prevents stampedes and compressive asphyxiation at high-density religious gatherings.\n• Dignity & Privacy: Protects women, sadhus, and families from intrusive optical surveillance and facial databases.\n• Calmer Pilgrimages: Controlled laminar crowd flow reduces panic, dehydration, and heat exhaustion in queues.",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• Grassroots Usability: Quad-coded visual alerts (Color + Icon + Word + Shape) and loud acoustic buzzers ensure ground marshals of all literacy levels can respond instantly.\n• Low Infrastructure Overhead: Ultra-low power (<2.5W) node can operate on solar-battery packs in remote shrines without high-bandwidth internet.",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "• Hardware Failsafe: Spring-release mechanical interlock ensures barriers never jam closed during power loss or evacuation.\n• Non-Coercive Management: Gentle metering at corridor entry prevents panic surges further down the line.\n• Minimal Carbon Footprint: ESP32 consumes 99% less power than high-end GPU video AI servers.",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 22':
        update_text(s,
            "• Phase 1 (Pilots): Entry barricades at Puri Jagannath Temple & Lingaraj Temple during festive seasons.\n• Phase 2 (Mega-Events): Cuttack Bali Yatra riverbank pavilions, Barabati Stadium entry gates, and Bhubaneswar Railway FOBs.\n• Phase 3 (Institutional): Standardized crowd safety hardware kit adopted by Odisha Police & District Collectors.",
            font_size=9, color=COLOR_DARK)
    elif s.name == 'TextBox 24':
        update_text(s,
            "Odisha hosts some of the world's largest religious congregations—over 1.5 million devotees gather for Puri Ratha Yatra alone, packing ancient, narrow heritage streets under intense monsoon humidity. Crowd Pressure Radar is purpose-built for Odisha's unique cultural geography: affordable, privacy-respecting, monsoon-resilient, and instantly actionable.",
            font_size=10.5, color=COLOR_DARK)

# ==============================================================================
# SLIDE 8: 07 • TEAM & NEXT STEPS
# ==============================================================================
slide8 = prs.slides[7]
for s in slide8.shapes:
    if s.name == 'TextBox 10':
        update_text(s,
            "• Technical & Embedded Lead: Expert in ESP32 firmware, 24 GHz mmWave radar DSP, WebSocket streaming, and hardware prototype design.\n• Product & UX Lead: Specializing in safety-critical user interfaces, Swiss Minimalist design systems, WCAG 2.2 accessibility, and public safety workflows.",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 14':
        update_text(s,
            "• Milestone 1 (Completed): Embedded firmware with 4-factor risk algorithm, Wokwi simulation, OLED/LED/servo integration.\n• Milestone 2 (Completed): High-performance Node.js telemetry server with 1 Hz WebSocket streaming and state machine hysteresis.\n• Milestone 3 (Completed): Operator dashboard with 2-second visual triage, live radar scope, and keyboard accessibility.\n• Milestone 4 (Post-Hackathon): 3D IP65 outdoor enclosure fabrication and field validation with Odisha police queue barricades.",
            font_size=9.5, color=COLOR_DARK)
    elif s.name == 'TextBox 16':
        update_text(s,
            "• Mentorship & Domain Validation: Guidance from Odisha Police crowd management specialists and SJTA temple security engineers.\n• Pilot Testing Site: Access to deploy a 3-node test corridor at Puri Badadanda or Lingaraj Temple queue barricades.\n• Incubation Support: Hardware prototyping grants and fabrication facilities through SOA–CII / Startup Odisha.",
            font_size=10.5, color=COLOR_DARK)
    elif s.name == 'TextBox 18':
        update_text(s,
            "“Crowd Pressure Radar empowers festival safety officers in Odisha to detect and prevent fatal crowd crushes up to 60 seconds early using anonymous 24 GHz edge radar intelligence without cameras.”",
            font_size=15, bold=True, color=COLOR_DARK)
    elif s.name == 'TextBox 19':
        update_text(s,
            "Thank you  •  Crowd Pressure Radar (RadarShield AI)  •  Odisha AI Hackathon 2026",
            font_size=12, color=COLOR_SLATE)

# Save updated presentation
output_pptx = r"C:\Users\subha\Downloads\crowd-pressure-radar\Odisha_AI_Hackathon_Idea_Pitchdeck_Template_White_Theme.pptx"
prs.save(output_pptx)
print(f"\nFinal polished pitchdeck saved to: {output_pptx}")
