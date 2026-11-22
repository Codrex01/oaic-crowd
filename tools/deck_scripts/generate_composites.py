import os
from PIL import Image, ImageDraw, ImageFont

assets_dir = r"C:\Users\subha\Downloads\crowd-pressure-radar\pitch_assets"
prototype_img_path = r"C:\Users\subha\Downloads\crowd-pressure-radar\prototype_render.jpg"
wokwi_img_path = r"C:\Users\subha\.gemini\antigravity\brain\6f3a8bf1-4c2b-4d07-b1a6-78e0e921ebee\.user_uploaded\media_1795023786550_1022c2ca.jpg"
dashboard_img_path = r"C:\Users\subha\Downloads\crowd-pressure-radar\screenshot_detail_1280.png"

# 1. Slide 1 Hero Prototype: Crop and polish 3D prototype render
if os.path.exists(prototype_img_path):
    proto_im = Image.open(prototype_img_path).convert("RGBA")
    # Add a clean border or subtle rounded aesthetic
    hero_out = os.path.join(assets_dir, "slide1_hero_prototype.png")
    proto_im.save(hero_out, "PNG")
    print(f"Created: {hero_out}")

# 2. Slide 6 Live Proof Composite: Wokwi Hardware + Live Dashboard
if os.path.exists(wokwi_img_path) and os.path.exists(dashboard_img_path):
    wokwi_im = Image.open(wokwi_img_path).convert("RGBA")
    dash_im = Image.open(dashboard_img_path).convert("RGBA")
    
    # Create side-by-side composite: 1600 x 900
    comp = Image.new("RGBA", (1600, 900), (255, 255, 255, 255))
    
    # Resize dashboard to fit left side (950 x 850)
    dash_resized = dash_im.resize((960, 600), Image.Resampling.LANCZOS)
    comp.paste(dash_resized, (30, 150))
    
    # Resize wokwi hardware to fit right side (550 x 750)
    # Crop wokwi to the circuit portion (center)
    w_w, w_h = wokwi_im.size
    wokwi_cropped = wokwi_im.crop((int(w_w * 0.35), int(w_h * 0.05), int(w_w * 0.85), int(w_h * 0.95)))
    wokwi_resized = wokwi_cropped.resize((540, 700), Image.Resampling.LANCZOS)
    comp.paste(wokwi_resized, (1020, 100))
    
    draw = ImageDraw.Draw(comp)
    # Header bar
    draw.rectangle([(0, 0), (1600, 70)], fill=(36, 45, 57))
    draw.text((40, 20), "LIVE WORKING PROOF: WEB OPERATOR DASHBOARD + EMBEDDED ESP32 NODE", fill=(255, 255, 255))
    
    # Labels
    draw.rectangle([(30, 770), (990, 850)], fill=(241, 245, 249), outline=(226, 232, 240))
    draw.text((50, 785), "Operator Dashboard: Live 24GHz mmWave radar target tracking, 10-min trend, physical gate control", fill=(71, 85, 105))
    
    draw.rectangle([(1020, 810), (1560, 870)], fill=(240, 253, 244), outline=(134, 239, 172))
    draw.text((1040, 825), "Wokwi Embedded Node: SSD1306 OLED, SG90 Servo, Green LED", fill=(21, 128, 61))

    proof_out = os.path.join(assets_dir, "slide6_live_proof.png")
    comp.save(proof_out, "PNG")
    print(f"Created: {proof_out}")
