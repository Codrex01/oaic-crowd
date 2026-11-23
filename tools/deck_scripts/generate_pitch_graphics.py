import os
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

os.makedirs(r"C:\Users\subha\Downloads\crowd-pressure-radar\pitch_assets", exist_ok=True)
assets_dir = r"C:\Users\subha\Downloads\crowd-pressure-radar\pitch_assets"

# Colors matching the presentation template
COLOR_CHARCOAL = "#242D39"
COLOR_SLATE = "#637080"
COLOR_GREEN = "#57AE30"
COLOR_RED = "#E61437"
COLOR_TEAL = "#1EAFA6"
COLOR_GOLD = "#F6B834"
COLOR_BG = "#FFFFFF"
COLOR_CARD_BG = "#F8FAFC"
COLOR_BORDER = "#E2E8F0"

plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.sans-serif'] = ['Segoe UI', 'DejaVu Sans', 'Arial', 'Helvetica']

# ==============================================================================
# 1. DONUT CHART: 4-Factor Physics Risk Formula Weights
# ==============================================================================
def create_weights_donut():
    fig, ax = plt.subplots(figsize=(6.2, 4.2), subplot_kw=dict(aspect="equal"), facecolor=COLOR_BG)
    
    weights = [40, 30, 20, 10]
    labels = [
        'Crowd Density (40%)\nNet inflow & area density',
        'Velocity Stall (30%)\nRadial walking speed <0.15 m/s',
        'Stop-and-Go Variance (20%)\n20s speed standard dev',
        'Spatial Clustering (10%)\nInter-person pairs <0.5m'
    ]
    colors = [COLOR_GREEN, COLOR_RED, COLOR_TEAL, COLOR_GOLD]
    explode = (0.02, 0.04, 0.02, 0.02)
    
    wedges, texts, autotexts = ax.pie(
        weights, 
        explode=explode, 
        colors=colors, 
        autopct='%1.0f%%',
        pctdistance=0.76,
        startangle=140,
        wedgeprops=dict(width=0.36, edgecolor='#FFFFFF', linewidth=2)
    )
    
    plt.setp(autotexts, size=11, weight="bold", color="white")
    
    # Inner circle text
    ax.text(0, 0.12, "COMPOSITE\nRISK ENGINE", ha='center', va='center', fontsize=11, fontweight='bold', color=COLOR_CHARCOAL)
    ax.text(0, -0.22, "0 – 100 Scale\n<12ms Edge Latency", ha='center', va='center', fontsize=9, color=COLOR_SLATE)
    
    # Legend
    ax.legend(
        wedges, labels,
        title="4 Physical Sensor Indicators",
        title_fontsize=10,
        loc="center left",
        bbox_to_anchor=(1.0, 0, 0.5, 1),
        fontsize=9,
        frameon=True,
        facecolor=COLOR_CARD_BG,
        edgecolor=COLOR_BORDER
    )
    
    plt.title("ESP32 Multi-Factor Physics Weight Distribution", fontsize=12, fontweight='bold', color=COLOR_CHARCOAL, pad=12)
    plt.tight_layout()
    out_path = os.path.join(assets_dir, "weights_donut.png")
    plt.savefig(out_path, dpi=300, bbox_inches='tight', facecolor=COLOR_BG)
    plt.close()
    print(f"Created: {out_path}")

# ==============================================================================
# 2. CROWD CRUSH DYNAMICS & CRUSH POINT COMPARISON (Slide 3)
# ==============================================================================
def create_crush_dynamics_chart():
    fig, ax = plt.subplots(figsize=(7.5, 3.8), facecolor=COLOR_BG)
    
    density = np.linspace(0.5, 7.0, 200)
    # Speed drops non-linearly as density increases (Greenshields/Weidmann model)
    speed = np.maximum(0.05, 1.4 * (1 - (density / 6.0)**1.3))
    
    # Pressure rises exponentially past 4 ppl/m²
    diff = np.maximum(0, density - 3.5)
    pressure = np.where(density < 3.5, 100 * (density / 3.5), 100 + 350 * (diff**1.6))
    
    ax.axvspan(0.5, 2.0, color='#F0FDF4', alpha=0.9, label='Free Flow (Normal)')
    ax.axvspan(2.0, 4.0, color='#FFFBEB', alpha=0.9, label='Congested Flow (Warning)')
    ax.axvspan(4.0, 7.0, color='#FEF2F2', alpha=0.9, label='Fatal Crush Zone (Turbulent Shockwaves)')
    
    line1, = ax.plot(density, speed, color=COLOR_TEAL, linewidth=2.5, label='Walking Velocity (m/s)')
    ax.set_xlabel("Crowd Density (people per m²)", fontsize=10, fontweight='bold', color=COLOR_CHARCOAL)
    ax.set_ylabel("Average Movement Speed (m/s)", fontsize=10, fontweight='bold', color=COLOR_TEAL)
    ax.set_ylim(0, 1.6)
    ax.tick_params(colors=COLOR_SLATE, labelsize=9)
    
    # Secondary axis for compressive force
    ax2 = ax.twinx()
    line2, = ax2.plot(density, pressure, color=COLOR_RED, linewidth=2.5, linestyle='--', label='Compressive Force (N)')
    ax2.set_ylabel("Physical Crowd Pressure (Newtons)", fontsize=10, fontweight='bold', color=COLOR_RED)
    ax2.set_ylim(0, 1200)
    ax2.tick_params(colors=COLOR_SLATE, labelsize=9)
    
    # Annotations
    ax.annotate('CCTV Optical Occlusion Blindspot\n(Bodies merge, vision fails)', 
                xy=(4.2, 0.2), xytext=(4.3, 0.75),
                arrowprops=dict(facecolor=COLOR_RED, shrink=0.05, width=1.2, headwidth=6),
                fontsize=8.5, fontweight='bold', color=COLOR_RED,
                bbox=dict(boxstyle="round,pad=0.3", fc="#FFF1F2", ec=COLOR_RED, lw=1))
    
    ax.annotate('mmWave Radar Tracks Micro-Doppler\n(Through dust, smoke & darkness)', 
                xy=(2.8, 0.8), xytext=(1.0, 1.25),
                arrowprops=dict(facecolor=COLOR_GREEN, shrink=0.05, width=1.2, headwidth=6),
                fontsize=8.5, fontweight='bold', color=COLOR_GREEN,
                bbox=dict(boxstyle="round,pad=0.3", fc="#F0FDF4", ec=COLOR_GREEN, lw=1))

    plt.title("The Fatal Transition: Velocity Collapse & Physical Pressure Escalation", fontsize=11, fontweight='bold', color=COLOR_CHARCOAL, pad=10)
    
    # Combined legend
    lines = [line1, line2]
    labels = [l.get_label() for l in lines]
    ax.legend(lines, labels, loc='center left', fontsize=8.5, frameon=True, facecolor=COLOR_CARD_BG, edgecolor=COLOR_BORDER)
    
    plt.tight_layout()
    out_path = os.path.join(assets_dir, "crush_dynamics.png")
    plt.savefig(out_path, dpi=300, bbox_inches='tight', facecolor=COLOR_BG)
    plt.close()
    print(f"Created: {out_path}")

# ==============================================================================
# 3. END-TO-END ARCHITECTURE FLOWCHART (Slide 4)
# ==============================================================================
def create_architecture_diagram():
    fig, ax = plt.subplots(figsize=(10.0, 3.2), facecolor=COLOR_BG)
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 3.2)
    ax.axis('off')
    
    # 4 Main Blocks
    blocks = [
        {"x": 0.3, "w": 2.0, "title": "1. SENSORY INPUT", "sub": "24 GHz mmWave Radar\n• HLK-LD2450 FMCW\n• Dual IR Beams (Entry/Exit)\n• Zero cameras / 100% anonymous", "color": COLOR_GREEN},
        {"x": 2.7, "w": 2.1, "title": "2. EDGE ML & DSP", "sub": "ESP32 (240MHz Dual-Core)\n• Micro-Doppler velocity\n• 20s rolling variance\n• 4-factor risk algorithm\n• State Machine (Hysteresis)", "color": COLOR_TEAL},
        {"x": 5.2, "w": 2.1, "title": "3. PHYSICAL INTERLOCK", "sub": "Deterministic Sub-10ms Action\n• SG90 Servo Gate (90° Lock)\n• Piezo Acoustic Siren\n• SSD1306 Local OLED\n• Quad-Coded Status LEDs", "color": COLOR_RED},
        {"x": 7.7, "w": 2.0, "title": "4. COMMAND DASHBOARD", "sub": "Central Station & SDRF\n• Node.js WebSocket (<80ms)\n• Swiss Minimalist React UI\n• 10-Min Predictive Trends\n• Safety Audit Logbook", "color": COLOR_GOLD}
    ]
    
    for b in blocks:
        # Card background
        rect = patches.FancyBboxPatch((b["x"], 0.3), b["w"], 2.4, boxstyle="round,pad=0.08,rounding_size=0.15",
                                      facecolor=COLOR_CARD_BG, edgecolor=b["color"], linewidth=2)
        ax.add_patch(rect)
        # Header banner
        header_rect = patches.FancyBboxPatch((b["x"], 2.25), b["w"], 0.45, boxstyle="round,pad=0.08,rounding_size=0.15",
                                             facecolor=b["color"], edgecolor=b["color"], linewidth=1)
        ax.add_patch(header_rect)
        ax.text(b["x"] + b["w"]/2, 2.47, b["title"], ha='center', va='center', fontsize=9, fontweight='bold', color='#FFFFFF')
        ax.text(b["x"] + 0.1, 1.25, b["sub"], ha='left', va='center', fontsize=8, color=COLOR_CHARCOAL, linespacing=1.35)
        
    # Arrows between blocks
    for x_arrow in [2.35, 4.85, 7.35]:
        ax.annotate('', xy=(x_arrow + 0.3, 1.5), xytext=(x_arrow, 1.5),
                    arrowprops=dict(arrowstyle="->", color=COLOR_SLATE, lw=2.5, mutation_scale=15))

    out_path = os.path.join(assets_dir, "architecture_flowchart.png")
    plt.savefig(out_path, dpi=300, bbox_inches='tight', facecolor=COLOR_BG)
    plt.close()
    print(f"Created: {out_path}")

# ==============================================================================
# 4. BENCHMARK / COMPARISON TABLE CHART (Slide 6 or Slide 7)
# ==============================================================================
def create_comparison_graphic():
    fig, ax = plt.subplots(figsize=(6.5, 3.2), facecolor=COLOR_BG)
    ax.axis('off')
    
    data = [
        ["Feature / Capability", "Optical CCTV / Vision AI", "Manual Police Whistles", "Crowd Pressure Radar"],
        ["Privacy / Anonymity", "Compromised (Facial PII)", "No data recorded", "100% Anonymous (RF Only)"],
        ["High Density (>4 p/m²)", "Severe body occlusion", "Human line-of-sight blocked", "Penetrates crowd bodies"],
        ["Monsoon Rain & Glare", "Lens blur & false alarms", "Severely restricted visibility", "24GHz FMCW all-weather"],
        ["Automated Interlock", "Notification only (delay)", "Whistles & physical gates", "Automatic servo lock <2.5s"],
        ["Edge Node Cost", "Rs. 40,000 - Rs. 1,20,000", "Manual staffing overhead", "Under Rs. 2,500 ($30 USD)"]
    ]
    
    table = ax.table(cellText=data, loc='center', cellLoc='left')
    table.auto_set_font_size(False)
    table.set_fontsize(8.5)
    table.scale(1.0, 1.45)
    
    # Style header row
    for col in range(4):
        cell = table[(0, col)]
        cell.set_facecolor(COLOR_CHARCOAL)
        cell.set_text_props(color='white', fontweight='bold')
        
    # Highlight CPR column
    for row in range(1, 6):
        cell_cpr = table[(row, 3)]
        cell_cpr.set_facecolor('#F0FDF4')
        cell_cpr.set_text_props(color='#15803D', fontweight='bold')
        
    plt.title("Technology Benchmark: Traditional vs. Radar Edge Architecture", fontsize=11, fontweight='bold', color=COLOR_CHARCOAL, pad=10)
    plt.tight_layout()
    out_path = os.path.join(assets_dir, "technology_benchmark.png")
    plt.savefig(out_path, dpi=300, bbox_inches='tight', facecolor=COLOR_BG)
    plt.close()
    print(f"Created: {out_path}")

create_weights_donut()
create_crush_dynamics_chart()
create_architecture_diagram()
create_comparison_graphic()
print("All pitch deck visual assets successfully generated!")
